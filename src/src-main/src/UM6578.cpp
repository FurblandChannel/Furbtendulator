#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "Debugger.h"
#include "GFX.h"
#include "NES.h"
#include "CPU.h"
#include "APU.h"
#include "PPU.h"
#include "UM6578.h"

uint8_t um6578extraRAM[2048]; // UM6578 has extra RAM at $5000-$57FF

namespace CPU {
CPU_UM6578::CPU_UM6578(uint8_t* _RAM, uint8_t* _RAM2):
	CPU_RP2A03(0, 8192, _RAM),
	RAM2(_RAM2) {
}

void	CPU_UM6578::PowerOn (void) {
	CPU_RP2A03::PowerOn();
	CPU::RandomMemory(RAM2, sizeof(RAM2));
}
	
int	CPU_UM6578::ReadRAM2 (int Bank, int Addr) {	
	return (Addr &0x800)? 0xFF: RAM2[Addr &0x7FF];
}

void	CPU_UM6578::WriteRAM2 (int Bank, int Addr, int Val) {
	if (~Addr &0x800) RAM2[Addr &0x7FF] = (unsigned char)Val;
}

int	CPU_UM6578::Save (FILE *out) {
	int clen =CPU_RP2A03::Save(out);
	writeArray(RAM2, 0x0800);
	return clen;
}

int	CPU_UM6578::Load (FILE *in, int version_id) {
	int clen =CPU_RP2A03::Load(in, version_id);
	readArray(RAM2, 0x0800);
	return clen;
}

int	MAPINT	ReadRAM2 (int Bank, int Addr) {
	CPU_UM6578* cpu =dynamic_cast<CPU_UM6578*>(CPU[Bank >>4]);
	return cpu->ReadRAM2(Bank, Addr);
}

void	MAPINT	WriteRAM2 (int Bank, int Addr, int Val) {
	CPU_UM6578* cpu =dynamic_cast<CPU_UM6578*>(CPU[Bank >>4]);
	cpu->WriteRAM2(Bank, Addr, Val);
}
} // namespace CPU

namespace PPU {
int	PPU_UM6578::IntRead (int bank, int addr) {
	if (addr ==0x008)
		return reg2008;
	else
	if (addr >=0x040 && addr <=0x07F)
		return Palette[addr &0x3F];
	else
		return PPU_RP2C02::IntRead(bank, addr);
}

void	PPU_UM6578::IntWrite (int bank, int addr, int val) {
	if (addr ==0x008) {
		reg2008 =val;
		colorMask =reg2008 &0x80? 0xF: 0x3;
	} else
	if (addr >=0x040 && addr <=0x07F) {
		Palette[addr &0x3F] =val;
		#ifdef	ENABLE_DEBUGGER
		Debugger::PalChanged =TRUE;
		#endif	/* ENABLE_DEBUGGER */
	} else
	if (addr <0x008) // PPU registers are not mirrored throughout the 2008-3FFF range
		PPU_RP2C02::IntWrite(bank, addr, val);
}

void	__fastcall	PPU_UM6578::Write6 (int Val) { // Allow bit 15
	if (HVTog) {
		IntReg &=0x00FF;
		IntReg |=(Val &0xFF) << 8;
	} else {
		IntReg &=0xFF00;
		IntReg |=Val;
		#if DELAY_VRAM_ADDRESS_CHANGE
		if (!IsRendering)
			VRAMAddr =IntReg;
		else
			UpdateVRAMAddr =3;
		#else
		VRAMAddr =IntReg;
		#endif
	}
	HVTog =!HVTog;
}

int	__fastcall PPU_UM6578::Read7 (void) { // No special treatment of $3Fxx necessary
	IOMode =5;
	return readLatch =buf2007;
}

void	__fastcall PPU_UM6578::Write7 (int Val) { // No special treatment of $3Fxx necessary
	IOVal =(unsigned char)Val;
	IOMode =6;
}

void	PPU_UM6578::RunNoSkip (int NumTicks) {
	register unsigned long TL;
	register unsigned char TC;
	register unsigned char BG;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i, y;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;
		
		if (Clockticks ==256) {
			if (SLnum <240)	ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 && Clockticks <=303) {
			if (IsRendering && SLnum ==-1) {
				VRAMAddr &=~0xFBE0;
				VRAMAddr |=IntReg &0xFBE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick)
					EndSLTicks =340;
				else	
					EndSLTicks =341;
			} else	
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			}
			if (SLnum ==SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[0]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				if (which ==NES::WhichScreenToShow) GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline
		
		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[Clockticks >>1 &3] = 0;	// seems to force black in this case
					if (IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
				} else
				if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData [Clockticks >>1 &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				else
					RenderData [Clockticks >>1 &3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				RenderAddr = reg2008 <<13 &0x2000 | VRAMAddr <<1 &0x1FFE; // Fetch low byte of name table word
				// Enforce one-table mirroring when using the $2xxx bank for nametable.
				// It is enforced during rendering reads, but not during writes (direct or DMA), so must be applied here!
				if (reg2008 &0x01) RenderAddr &=~0x1800;
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				// Set up low 12 bits of the CHR address
				PatAddr =RenderData[0] <<4 | VRAMAddr >>12;
				if (reg2008 &0x80) PatAddr &=~0x10; // Ignore lowest bit of tile number in 4bpp mode
				break;
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr++; // Fetch attribute and high byte of name table
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
				// Set up the high 4 bits of the CHR address
				PatAddr |=RenderData[1] <<12 &0xF000;
				CurTileData = &TileData[Clockticks + 13];
				BG =RenderData[1] >>4 &(reg2008 &0x80? 0xC: 0xF);
				TL = BG * 0x04040404;
				
				((unsigned long *)CurTileData)[0] = TL;
				((unsigned long *)CurTileData)[1] = TL;
				break;
			case 323:	case 331:
				// Set up the high 4 bits of the CHR address
				PatAddr |=RenderData[1] <<12 &0xF000;

				CurTileData = &TileData[Clockticks - 323];
				BG =RenderData[1] >>4 &(reg2008 &0x80? 0xC: 0xF);
				TL = BG * 0x04040404;				
				((unsigned long *)CurTileData)[0] = TL;
				((unsigned long *)CurTileData)[1] = TL;
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				RenderAddr =PatAddr;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				TC = ReverseCHR[RenderData[2]]; // CHR-ROM data planes 0-1
				CurTileData = &TileData[Clockticks + 11];
				((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];
				
				if (reg2008 &0x80) { // 4bpp
					// CHR-ROM data planes 2-3
					RenderAddr |=0x10;
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
	
					TC = ReverseCHR[RenderDataHi[2]];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<2;
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<2;
				}
				break;
			case 325:	case 333:
				TC = ReverseCHR[RenderData[2]];
				CurTileData = &TileData[Clockticks - 325];
				((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];

				if (reg2008 &0x80) { // 4bpp
					// CHR-ROM data planes 2-3
					RenderAddr |=0x10;
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
	
					TC = ReverseCHR[RenderDataHi[2]];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<2;
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<2;
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr = PatAddr | 8;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				TC = ReverseCHR[RenderData[3]]; // CHR-ROM data planes 0-1
				CurTileData = &TileData[Clockticks + 9];
				((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];

				if (reg2008 &0x80) { // 4bpp
					// CHR-ROM data planes 2-3
					RenderAddr |=0x10;
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);

					TC = ReverseCHR[RenderDataHi[3]];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<2;
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<2;
				}
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				TC = ReverseCHR[RenderData[3]]; // CHR-ROM data planes 0-1
				CurTileData = &TileData[Clockticks - 327];
				((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];

				if (reg2008 &0x80) { // 4bpp
					// CHR-ROM data planes 2-3
					RenderAddr |=0x10;
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);

					TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data planes 2-3
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<2;
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<2;
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &= ~0x41F;
				VRAMAddr |= IntReg & 0x41F;
					case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum = (Clockticks >> 1) & 0x1C;
				TC = SprBuff[SprNum | 1];
				SprSL = (unsigned char)(SLnum - SprBuff[SprNum]);
 				if (Reg2000 & 0x20) // 8x16 sprites
					PatAddr = ((TC & 0xFE) << 4) | ((TC & 0x01) << 12) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x17 : 0x00) ^ ((SprSL & 0x8) << 1) | reg2008 <<10 &0x2000);
				else // 8x8 sprites
					PatAddr = (TC << 4) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x7 : 0x0) | reg2008 <<10 &0x3000);

				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr = PatAddr;
				break;
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				SprNum = (Clockticks >> 1) & 0x1E;
				if (SprBuff[SprNum] &0x40)
					TC = RenderData[2];
				else	
					TC = ReverseCHR[RenderData[2]];
				TL = (SprBuff[SprNum] & 0x3) * 0x04040404;
				CurTileData = SprData[SprNum >> 2];
				((unsigned long *)CurTileData)[0] = CHRLoBit[TC & 0xF] | TL;
				((unsigned long *)CurTileData)[1] = CHRLoBit[TC >> 4] | TL;				
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr = PatAddr | 8;
				break;
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				SprNum = (Clockticks >> 1) & 0x1E;
				if (SprBuff[SprNum] & 0x40)
					TC = RenderData[3];
				else	TC = ReverseCHR[RenderData[3]];
				CurTileData = SprData[SprNum >> 2];
				((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
				CurTileData[16] = SprBuff[SprNum];
				CurTileData[17] = SprBuff[SprNum | 1];
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (!(Clockticks & 1)) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2)
					WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, RenderAddr & 0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr = (unsigned short)(VRAMAddr & 0xFFFF);
			if ((IOMode >= 5) && (!IsRendering))
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else if (IOMode == 2) {
				if (!IsRendering)
					WriteHandler[addr >> 10](addr >> 10, addr & 0x3FF, IOVal);
			}
			else if (IOMode == 1)
			{
				IOMode++;
				if (!IsRendering)
				{
					if (ReadHandler[addr >> 10] == BusRead)
						buf2007 = CHRPointer[addr >> 10][addr & 0x3FF];
					else	buf2007 = (unsigned char)ReadHandler[addr >> 10](addr >> 10, addr & 0x3FF);
				}
			}
			IOMode -= 2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks == 255)) IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!((Clockticks & 7) == 7 && !(255 <= Clockticks && Clockticks <= 319))) IncrementH();
				} else	
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);
		
		register int PalIndex, DisplayedTC;
		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 &0x02))
				TC =TileData[Clockticks +IntX];
			else	
				TC =0;
			DisplayedTC =showBG? TC: 0;
			
			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (y =0; y <SprCount; y +=4) {
				register int SprPixel =Clockticks -SprData[y >>2][17];
				register unsigned char SprDat;
				if (SprPixel &~7) continue;
				SprDat =SprData[y >>2][SprPixel];
				if (SprDat &0x3) {
					if (Spr0InLine && y ==0 && Clockticks <255) {
						Spr0Hit =1;
						Spr0InLine =FALSE;
					}
					if (showOBJ && !(TC &colorMask && SprData[y >> 2][16] &0x20)) DisplayedTC =SprDat |0x10;
					break;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
		if (SLnum <242) {
			if (Clockticks ==325)
				PalIndex =Palette[0x00] &0x30;
			else
				PalIndex =Palette[0x00];
		}
		PalIndex &=GrayScale;
		PalIndex |= ColorEmphasis;
		*GfxData++ =PalIndex;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) *GfxData++ =PalIndex;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

void	PPU_UM6578::RunSkip (int NumTicks) {
	register unsigned char TC;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;
		
		if (Clockticks ==256) {
			if (SLnum <240 && Spr0InLine) ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 && Clockticks <=303) {
			if (IsRendering && SLnum ==-1) {
				VRAMAddr &=~0xFBE0;
				VRAMAddr |=IntReg &0xFBE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick)
					EndSLTicks =340;
				else	
					EndSLTicks =341;
			} else	
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			}
			if (SLnum ==SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[0]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				if (which ==NES::WhichScreenToShow) GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline
		
		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[Clockticks >>1 &3] = 0;	// seems to force black in this case
					if (IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
				} else
				if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData [Clockticks >>1 &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				else
					RenderData [Clockticks >>1 &3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				RenderAddr = reg2008 <<13 &0x2000 | VRAMAddr <<1 &0x1FFE; // Fetch low byte of name table word
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				// Set up low 12 bits of the CHR address
				PatAddr =RenderData[0] <<4 | VRAMAddr >>12;
				if (reg2008 &0x80) PatAddr &=~0x10; // Ignore lowest bit of tile number in 4bpp mode
				break;
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr++; // Fetch attribute and high byte of name table
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			case 323:	case 331:
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				RenderAddr =PatAddr;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				if (Spr0InLine)	{
					TC = ReverseCHR[RenderData[2]]; // CHR-ROM data planes 0-1
					CurTileData = &TileData[Clockticks + 11];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];
					
					if (reg2008 &0x80) { // 4bpp
						// CHR-ROM data planes 2-3
						RenderAddr |=0x10;
						if (ReadHandler[RenderAddr >>10] ==BusRead)
							RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
						else
							RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
		
						TC = ReverseCHR[RenderDataHi[2]];
						((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<2;
						((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<2;
					}
				}
				break;
			case 325:	case 333:
				if (Spr0InLine)	{
					TC = ReverseCHR[RenderData[2]];
					CurTileData = &TileData[Clockticks - 325];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];
	
					if (reg2008 &0x80) { // 4bpp
						// CHR-ROM data planes 2-3
						RenderAddr |=0x10;
						if (ReadHandler[RenderAddr >>10] ==BusRead)
							RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
						else
							RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
		
						TC = ReverseCHR[RenderDataHi[2]];
						((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<2;
						((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<2;
					}
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr = PatAddr | 8;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				if (Spr0InLine) {
					TC = ReverseCHR[RenderData[3]]; // CHR-ROM data planes 0-1
					CurTileData = &TileData[Clockticks + 9];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
	
					if (reg2008 &0x80) { // 4bpp
						// CHR-ROM data planes 2-3
						RenderAddr |=0x10;
						if (ReadHandler[RenderAddr >>10] ==BusRead)
							RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
						else
							RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
	
						TC = ReverseCHR[RenderDataHi[3]];
						((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<2;
						((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<2;
					}
				}
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				if (Spr0InLine) {
					TC = ReverseCHR[RenderData[3]]; // CHR-ROM data planes 0-1
					CurTileData = &TileData[Clockticks - 327];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
	
					if (reg2008 &0x80) { // 4bpp
						// CHR-ROM data planes 2-3
						RenderAddr |=0x10;
						if (ReadHandler[RenderAddr >>10] ==BusRead)
							RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
						else
							RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF);
	
						TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data planes 2-3
						((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<2;
						((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<2;
					}
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &= ~0x41F;
				VRAMAddr |= IntReg & 0x41F;
					case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum = (Clockticks >> 1) & 0x1C;
				TC = SprBuff[SprNum | 1];
				SprSL = (unsigned char)(SLnum - SprBuff[SprNum]);
 				if (Reg2000 & 0x20) // 8x16 sprites
					PatAddr = ((TC & 0xFE) << 4) | ((TC & 0x01) << 12) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x17 : 0x00) ^ ((SprSL & 0x8) << 1) | reg2008 <<10 &0x2000);
				else // 8x8 sprites
					PatAddr = (TC << 4) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x7 : 0x0) | reg2008 <<10 &0x3000);

				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr = PatAddr;
				break;
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				SprNum = (Clockticks >> 1) & 0x1E;
				if (Spr0InLine) {
					if (SprBuff[SprNum] &0x40)
						TC = RenderData[2];
					else	
						TC = ReverseCHR[RenderData[2]];
					CurTileData = SprData[SprNum >> 2];
					((unsigned long *)CurTileData)[0] = CHRLoBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] = CHRLoBit[TC >> 4];
				}
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr = PatAddr | 8;
				break;
			case 263:	
				if (Spr0InLine)	{
					SprNum = (Clockticks >> 1) & 0x1E;
					if (SprBuff[SprNum] & 0x40)
						TC = RenderData[3];
					else	TC = ReverseCHR[RenderData[3]];
					CurTileData = SprData[SprNum >> 2];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
					CurTileData[16] = SprBuff[SprNum];
					CurTileData[17] = SprBuff[SprNum | 1];
				}
				break;
			case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (!(Clockticks & 1)) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2)
					WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, RenderAddr & 0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr = (unsigned short)(VRAMAddr & 0xFFFF);
			if ((IOMode >= 5) && (!IsRendering))
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else if (IOMode == 2) {
				if (!IsRendering)
					WriteHandler[addr >> 10](addr >> 10, addr & 0x3FF, IOVal);
			}
			else if (IOMode == 1)
			{
				IOMode++;
				if (!IsRendering)
				{
					if (ReadHandler[addr >> 10] == BusRead)
						buf2007 = CHRPointer[addr >> 10][addr & 0x3FF];
					else	buf2007 = (unsigned char)ReadHandler[addr >> 10](addr >> 10, addr & 0x3FF);
				}
			}
			IOMode -= 2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks == 255)) IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!((Clockticks & 7) == 7 && !(255 <= Clockticks && Clockticks <= 319))) IncrementH();
				} else	
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);
		
		if (Spr0InLine && Clockticks <255 && OnScreen && (Reg2001 &0x18) ==0x18 && (Clockticks >=8 || Settings::PPUNeverClip || (Reg2001 &0x06) ==0x06)) {
			register int SprPixel =Clockticks -SprData[0][17];
			if (!(SprPixel &~7) &&(SprData[0][SprPixel] &0x3)) {
				//Reg2002 |=0x40;	// Sprite 0 hit
				Spr0Hit =1;
				Spr0InLine =FALSE;
			}
		}
		GfxData++;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) GfxData++;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

int	PPU_UM6578::GetPalIndex(int TC) {
	if (TC &colorMask)
		return Palette[TC &0x3F];
	else
		return Palette[0];
}

void PPU_UM6578::IncrementV () {
	if ((VRAMAddr &0x7000) ==0x7000) { // Fine Y-scroll now 7? Then next tile row
		VRAMAddr &=0x0FFF; // Set fine Y-scroll to zero
		if ((VRAMAddr &0x3E0) ==0x3E0) // Last tile row reached?
			VRAMAddr ^=0xBE0; // First tile row of nametable below the current one (nametable +2)			
		else
			VRAMAddr +=0x20; // Otherwise just next tile row
	} else
		VRAMAddr +=0x1000; // Increment fine Y-scroll
}

void	PPU_UM6578::IncrementAddr () {
	if (Reg2000 &0x04)
		VRAMAddr +=32;
	else	
		VRAMAddr++;
	VRAMAddr &=0xFFFF;
}

int	PPU_UM6578::Save (FILE *out) {
	int clen =PPU_RP2C02::Save(out);
	writeArray(&VRAM[4], 0x1800);
	writeArray(Palette+0x20, 0x40 -0x20);
	writeByte(reg2008);
	writeByte(colorMask);
	return clen;
}

int	PPU_UM6578::Load (FILE *in, int version_id) {
	int clen =PPU_RP2C02::Load(in, version_id);
	readArray(&VRAM[4], 0x1800);
	readArray(Palette+0x20, 0x40 -0x20);
	readByte(reg2008);
	readByte(colorMask);
	return clen;
}

} // namespace PPU

namespace APU {
void	APU_UM6578::Reset (void) {
	dmaControl =0;
	dmaPage =RI.INES2_SubMapper ==1? 0x20: 0x00;
	dmaSourceAddress =0;
	dmaTargetAddress =0;
	dmaLength =0;
	dmaBusyCount =0;
	APU_RP2A03::Reset();
	Bits =Regs[0x17] =0x40; // UM6578 boots with disabled frame IRQ, regardless of setting
}

int	APU_UM6578::IntRead (int bank, int addr) {
	switch(addr) {
		case 0x020:
			if (keyboardBuffer.empty())
				return 0xFF;
			else {
				int result =keyboardBuffer.front();
				keyboardBuffer.pop();
				return result;
			}
		case 0x033:
			return keyboardBuffer.size()? 0x20: 0x00;
		case 0x048:
			return dmaControl | (dmaBusyCount? 0x80: 0x00);
		case 0x049:
			return dmaPage;
		case 0x04A:
			return dmaSourceAddress      &0xFF;
		case 0x04B:
			return dmaSourceAddress >>8  &0xFF;
		case 0x04C:
			return dmaTargetAddress      &0xFF;
		case 0x04D:
			return dmaTargetAddress >>8  &0xFF;
		case 0x04E:
			return dmaLength     &0xFF;
		case 0x04F:
			return dmaLength >>8 &0xFF;
			break;
		default:
			return APU_RP2A03::IntRead(bank, addr);
	}
}

void	APU_UM6578::IntWrite (int bank, int addr, int val) {
	switch(addr) {
		case 0x016:
			if (RI.INES2_SubMapper ==0) dmaPage =dmaPage &0x1F | val <<4 &~0x1F;
			APU_RP2A03::IntWrite(bank, addr, val);
			break;
		case 0x026:
			if (RI.INES2_SubMapper ==1) dmaPage =dmaPage &0x1F | val >>2 &~0x1F;
			APU_RP2A03::IntWrite(bank, addr, val);
			break;			
		case 0x048:
			dmaControl =val;
			break;
		case 0x049:
			dmaPage =dmaPage &~0x1F | val &0x1F;
			break;
		case 0x04A:
			dmaSourceAddress =dmaSourceAddress &0xFF00 | val;
			break;
		case 0x04B:
			dmaSourceAddress =dmaSourceAddress &0x00FF | val <<8;
			break;
		case 0x04C:
			dmaTargetAddress =dmaTargetAddress &0xFF00 | val;
			break;
		case 0x04D:
			dmaTargetAddress =dmaTargetAddress &0x00FF | val <<8;
			break;
		case 0x04E:
			dmaLength =dmaLength &0xFF00 | val;
			break;
		case 0x04F:
			dmaLength =dmaLength &0x00FF | val <<8 &0x7F00;
			break;
		default:
			//if (addr >0x17) EI.DbgOut(L"%X%03X: %02X", bank, addr, val);
			APU_RP2A03::IntWrite(bank, addr, val);
			break;
	}
	if (dmaControl &0x80) {
		dmaControl &=~0x80; // Prevent recursive invocation in case of using DMA to write to APU registers (as NT6578V7 does)
		uint16_t oldSourceAddress =dmaSourceAddress;
		uint16_t oldTargetAddress =dmaTargetAddress;
		uint16_t oldLength        =dmaLength;
		/*
		if (dmaSourceAddress &0x8000) {
			uint32_t source = dmaSourceAddress &0xFFF |
			                 (EI.GetPRG_Ptr4(dmaSourceAddress >>12) -RI.PRGROMData) &0x7000 |
			                  dmaPage <<15;
			EI.DbgOut(L"DMA from ROM %05X->%s %04X x%04X", source &(RI.PRGROMSize -1), dmaControl &0x20? L"CPU": L"PPU", dmaTargetAddress, dmaLength);
		} else
			EI.DbgOut(L"DMA from RAM %04X->%s %04X x%04X", dmaSourceAddress, dmaControl &0x20? L"CPU": L"PPU", dmaTargetAddress, dmaLength);
		*/
		for (int i =0; i <=dmaLength; i++) {
			uint8_t byte;
			if (dmaSourceAddress &0x8000) {
				/*uint32_t source = dmaSourceAddress &0xFFF |
				                 (EI.GetPRG_Ptr4(dmaSourceAddress >>12) -RI.PRGROMData) &0x7000 |
				                  dmaPage <<15;*/
				uint32_t source =dmaSourceAddress &0x7FFF | dmaPage <<15;
				byte =RI.PRGROMData[source &(RI.PRGROMSize -1)];
			} else {
				byte =(EI.GetCPUReadHandler(dmaSourceAddress >>12))(dmaSourceAddress >>12, dmaSourceAddress &0xFFF);
			}
			if (dmaControl &0x20) // Work RAM
				(EI.GetCPUWriteHandler(dmaTargetAddress >>12))(dmaTargetAddress >>12, dmaTargetAddress &0xFFF, byte);
			else // VRAM
				(EI.GetPPUWriteHandler(dmaTargetAddress >>10))(dmaTargetAddress >>10, dmaTargetAddress &0x3FF, byte);
			dmaSourceAddress++;
			dmaTargetAddress++;
			// It's NECESSARY to return the "DMA Busy" status somewhat accurately for "Future Soldier" on ABL Wik!d Joystick 14-in-1
			dmaBusyCount +=dmaControl &0x40? 1: 2;
		}
		// "Race One" on "Handy Boy All-in-One Machine - Racing Games" depends on the registers retaining their written-to value after a transfer
		dmaSourceAddress =oldSourceAddress;
		dmaTargetAddress =oldTargetAddress;
		dmaLength        =oldLength;
	}
}

void	APU_UM6578::Run (void) {
	APU_RP2A03::Run();
	if (dmaBusyCount) dmaBusyCount--;
}

int	APU_UM6578::Save (FILE *out) {
	int clen =APU_RP2A03::Save(out);
	writeByte(dmaControl);
	writeByte(dmaPage);
	writeWord(dmaSourceAddress);
	writeWord(dmaTargetAddress);
	writeWord(dmaLength);
	return clen;
}

int	APU_UM6578::Load (FILE *in, int version_id) {
	int clen =APU_RP2A03::Load(in, version_id);
	readByte(dmaControl);
	readByte(dmaPage);
	readWord(dmaSourceAddress);
	readWord(dmaTargetAddress);
	readWord(dmaLength);
	return clen;
}

void    APU_UM6578::handleWindowMessage (UINT message, WPARAM wParam, LPARAM lParam) {
	/*switch(message) {
		case WM_KEYDOWN:
			keyboardBuffer.push(wParam &0x7F |0x00);
			EI.DbgOut(L"Key: %02X size=%d", wParam, keyboardBuffer.size());
			break;
		case WM_KEYUP:
			keyboardBuffer.push(wParam &0x7F |0x80);
			break;
	}*/
}
} // namespace APU