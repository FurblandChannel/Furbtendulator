#include	"..\..\DLL\d_iNES.h"

namespace {
FCPURead _Read[0x10];
FCPUWrite _Write[0x10];

uint8_t REG_WRAM_enabled, REG_map_ROM_on_6000;
uint8_t REG_WRAM_page, REG_can_write_CHR_RAM, REG_can_write_PRG;
uint8_t REG_flags, REG_mapper, REG_mirroring, REG_four_screen, REG_lockout;

uint16_t REG_PRG_base;
uint8_t REG_PRG_mask, REG_PRG_mode, REG_PRG_bank_6000, REG_PRG_bank_A, REG_PRG_bank_B, REG_PRG_bank_C, REG_PRG_bank_D;
uint32_t REG_PRG_bank_6000_mapped, REG_PRG_bank_A_mapped, REG_PRG_bank_B_mapped, REG_PRG_bank_C_mapped, REG_PRG_bank_D_mapped;

uint8_t REG_CHR_mask, REG_CHR_mode;
uint8_t REG_CHR_bank_A, REG_CHR_bank_B, REG_CHR_bank_C, REG_CHR_bank_D, REG_CHR_bank_E, REG_CHR_bank_F, REG_CHR_bank_G, REG_CHR_bank_H;
uint8_t REG_CHR_bank_A_alt, REG_CHR_bank_B_alt, REG_CHR_bank_C_alt, REG_CHR_bank_D_alt;
uint8_t ppu_latch0, ppu_latch1, ppu_mapper163_latch, TKSMIR[8];

uint8_t REG_scanline_IRQ_enabled, REG_scanline_IRQ_counter, REG_scanline_IRQ_latch, REG_scanline_IRQ_reload;
uint8_t REG_scanline2_IRQ_enabled, REG_scanline2_IRQ_line, REG_scanline2_IRQ_pending;

uint16_t REG_CPU_IRQ_value;
uint8_t REG_CPU_IRQ_control;
uint16_t REG_CPU_IRQ_latch;
uint8_t REG_VRC4_IRQ_prescaler;
uint8_t REG_VRC4_IRQ_prescaler_counter;

uint8_t REG_R0, REG_R1, REG_R2, REG_R3, REG_R4, REG_R5;
uint8_t REG_mul1, REG_mul2;
int LastPPUAddr, LastPPUScanline, LastPPUCycle, LastPPUIsRendering;

void	SyncPRG (void) {
	REG_PRG_bank_6000_mapped = (REG_PRG_base << 1) | (REG_PRG_bank_6000 & (((~REG_PRG_mask & 0x7F) << 1) | 1));
	REG_PRG_bank_A_mapped = (REG_PRG_base << 1) | (REG_PRG_bank_A & (((~REG_PRG_mask & 0x7F) << 1) | 1));
	REG_PRG_bank_B_mapped = (REG_PRG_base << 1) | (REG_PRG_bank_B & (((~REG_PRG_mask & 0x7F) << 1) | 1));
	REG_PRG_bank_C_mapped = (REG_PRG_base << 1) | (REG_PRG_bank_C & (((~REG_PRG_mask & 0x7F) << 1) | 1));
	REG_PRG_bank_D_mapped = (REG_PRG_base << 1) | (REG_PRG_bank_D & (((~REG_PRG_mask & 0x7F) << 1) | 1));
	switch (REG_PRG_mode & 7) {
		default:
		case 0:	EMU->SetPRG_ROM16(0x8, REG_PRG_bank_A_mapped >> 1);
			EMU->SetPRG_ROM16(0xC, REG_PRG_bank_C_mapped >> 1);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, REG_PRG_bank_C_mapped >> 1);
			EMU->SetPRG_ROM16(0xC, REG_PRG_bank_A_mapped >> 1);
			break;
		case 4:	EMU->SetPRG_ROM8(0x8, REG_PRG_bank_A_mapped);
			EMU->SetPRG_ROM8(0xA, REG_PRG_bank_B_mapped);
			EMU->SetPRG_ROM8(0xC, REG_PRG_bank_C_mapped);
			EMU->SetPRG_ROM8(0xE, REG_PRG_bank_D_mapped);
			break;
		case 5:	EMU->SetPRG_ROM8(0x8, REG_PRG_bank_C_mapped);
			EMU->SetPRG_ROM8(0xA, REG_PRG_bank_B_mapped);
			EMU->SetPRG_ROM8(0xC, REG_PRG_bank_A_mapped);
			EMU->SetPRG_ROM8(0xE, REG_PRG_bank_D_mapped);
			break;
		case 6:	EMU->SetPRG_ROM32(0x8, REG_PRG_bank_B_mapped >> 2);
			break;
		case 7:	EMU->SetPRG_ROM32(0x8, REG_PRG_bank_A_mapped >> 2);
			break;
	}
	if (REG_map_ROM_on_6000) 
		EMU->SetPRG_ROM8(0x6, REG_PRG_bank_6000_mapped);
	else {
		if (REG_WRAM_enabled)
			EMU->SetPRG_RAM8(0x6, REG_WRAM_page);
		else {
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
		}
	}
}

/*
	01	F7
	02	EF
	03	E7
	1F	07
*/

void	SetMappedCHR1 (int Bank, int Val) {
	int CHRMask =((((~REG_CHR_mask & 0x1F) + 1) * 0x2000) /0x400) -1;
	EMU->SetCHR_RAM1(Bank, Val &CHRMask);
	EMU->SetCHR_Ptr1(Bank, EMU->GetCHR_Ptr1(Bank), REG_can_write_CHR_RAM);
}
void	SetMappedCHR2 (int Bank, int Val) {
	for (int i =0; i <2; i++) SetMappedCHR1(Bank +i, Val *2 +i);
}
void	SetMappedCHR4 (int Bank, int Val) {
	for (int i =0; i <4; i++) SetMappedCHR1(Bank +i, Val *4 +i);
}
void	SetMappedCHR8 (int Bank, int Val) {
	for (int i =0; i <8; i++) SetMappedCHR1(Bank +i, Val *8 +i);
}

static void SyncCHR(void) {
	switch (REG_CHR_mode &7) {
		default:
		case 0:	SetMappedCHR8(0, REG_CHR_bank_A >> 3);
			break;
		case 1:	SetMappedCHR4(0, ppu_mapper163_latch);
			SetMappedCHR4(4, ppu_mapper163_latch);
			break;
		case 2:	SetMappedCHR2(0, REG_CHR_bank_A >> 1);	TKSMIR[0] = TKSMIR[1] = REG_CHR_bank_A;
			SetMappedCHR2(2, REG_CHR_bank_C >> 1);	TKSMIR[2] = TKSMIR[3] = REG_CHR_bank_C;
			SetMappedCHR1(4, REG_CHR_bank_E);	TKSMIR[4] = REG_CHR_bank_E;
			SetMappedCHR1(5, REG_CHR_bank_F);	TKSMIR[5] = REG_CHR_bank_F;
			SetMappedCHR1(6, REG_CHR_bank_G);	TKSMIR[6] = REG_CHR_bank_G;
			SetMappedCHR1(7, REG_CHR_bank_H);	TKSMIR[7] = REG_CHR_bank_H;
			break;
		case 3:	SetMappedCHR1(0, REG_CHR_bank_E);	TKSMIR[0] = REG_CHR_bank_E;
			SetMappedCHR1(1, REG_CHR_bank_F);	TKSMIR[1] = REG_CHR_bank_F;
			SetMappedCHR1(2, REG_CHR_bank_G);	TKSMIR[2] = REG_CHR_bank_G;
			SetMappedCHR1(3, REG_CHR_bank_H);	TKSMIR[3] = REG_CHR_bank_H;
			SetMappedCHR2(4, REG_CHR_bank_A >> 1);	TKSMIR[4] = TKSMIR[5] = REG_CHR_bank_A;
			SetMappedCHR2(6, REG_CHR_bank_C >> 1);	TKSMIR[6] = TKSMIR[7] = REG_CHR_bank_C;
			break;
		case 4:	SetMappedCHR4(0, REG_CHR_bank_A >> 2);
			SetMappedCHR4(4, REG_CHR_bank_E >> 2);
			break;
		case 5:	if (!ppu_latch0)
				SetMappedCHR4(0, REG_CHR_bank_A >> 2);
			else
				SetMappedCHR4(0, REG_CHR_bank_B >> 2);
			if (!ppu_latch1)
				SetMappedCHR4(4, REG_CHR_bank_E >> 2);
			else
				SetMappedCHR4(4, REG_CHR_bank_F >> 2);
			break;
		case 6:	SetMappedCHR2(0, REG_CHR_bank_A >> 1);
			SetMappedCHR2(2, REG_CHR_bank_C >> 1);
			SetMappedCHR2(4, REG_CHR_bank_E >> 1);
			SetMappedCHR2(6, REG_CHR_bank_G >> 1);
			break;
		case 7:	SetMappedCHR1(0, REG_CHR_bank_A);
			SetMappedCHR1(1, REG_CHR_bank_B);
			SetMappedCHR1(2, REG_CHR_bank_C);
			SetMappedCHR1(3, REG_CHR_bank_D);
			SetMappedCHR1(4, REG_CHR_bank_E);
			SetMappedCHR1(5, REG_CHR_bank_F);
			SetMappedCHR1(6, REG_CHR_bank_G);
			SetMappedCHR1(7, REG_CHR_bank_H);
			break;
	}
}

void	SyncMirroring (void) {
	if (REG_four_screen)
		EMU->Mirror_4();
	else if (!((REG_mapper ==20) && (REG_flags & 1))) switch (REG_mirroring) { // Mapper 189 check
		case 0:	EMU->Mirror_V(); break;
		case 1:	EMU->Mirror_H(); break;
		case 2: EMU->Mirror_S0(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

void	Sync (void) {
	SyncPRG();
	SyncCHR();
	SyncMirroring();
}

void	MAPINT	Write4 (int Bank, int Addr, int Val) {
	_Write[Bank](Bank, Addr, Val);
	
	// iNES Mapper #20: Famicom Disk System
	if (REG_mapper ==31 && Addr ==0x25) REG_mirroring = ((Val & 8) >> 3) & 1;

	// iNES Mapper #189: TXC 01-22018-400
	if (Addr >=0x20 && (REG_mapper ==20) && (REG_flags &2)) REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Val & 0x0F) << 2) | ((Val & 0xF0) >> 2);
	
	Sync();
}

int	MAPINT	Read5 (int Bank, int Addr) {
	if (REG_mapper ==6) { // iNES Mapper #163: 南晶
		if ((Addr &0x700) ==0x100) return REG_R2 | REG_R0 | REG_R1 | ~REG_R3;
		if ((Addr &0x700) ==0x500) return (REG_R5&1) ? REG_R2 : REG_R1;
	}
	if (REG_mapper ==15 && Addr ==0x204) { // MMC5
		uint8_t p = REG_scanline2_IRQ_pending;
		uint8_t r = (!LastPPUIsRendering || LastPPUScanline+1 >= 241) ? 0 : 1;
		EMU->SetIRQ(1);
		REG_scanline2_IRQ_pending = 0;
		return (p<<7) | (r<<6);
	}
	return (REG_mapper ==0)? 0: *EMU->OpenBus;
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	if (!REG_lockout) {	// iNES Mapper #342: Coolgirl
		switch (Addr &7) {
			case 0:	REG_PRG_base = (REG_PRG_base & 0xFF) | (Val << 8);
				break;
			case 1:	REG_PRG_base = (REG_PRG_base & 0xFF00) | Val;
				break;
			case 2: REG_PRG_mask = Val & 0xFF;
				break;
			case 3:	REG_PRG_mode = (Val & 0xE0) >> 5; 
				REG_CHR_bank_A = (REG_CHR_bank_A & 7) | (Val << 3);
				break;
			case 4:	REG_CHR_mode = (Val & 0xE0) >> 5;
				REG_CHR_mask = Val & 0x1F;
				break;
			case 5:	REG_PRG_bank_A = (REG_PRG_bank_A & 0xC1) | ((Val & 0x7C) >> 1);
				REG_WRAM_page = Val&3;
				break;
			case 6:	REG_flags = (Val & 0xE0) >> 5;
				REG_mapper = Val & 0x1F;
				break;
			case 7:	REG_lockout = (Val & 0x80) >> 7;
				REG_four_screen = (Val & 0x20) >> 5;
				REG_mirroring = (Val & 0x18) >> 3;
				REG_can_write_PRG = (Val & 4) >> 2;
				REG_can_write_CHR_RAM = (Val & 2) >> 1;
				REG_WRAM_enabled = Val & 1;
				break;
		}
		if (REG_mapper ==17) REG_PRG_bank_B = 0xFD; // for MMC2
		if (REG_mapper ==14) REG_PRG_bank_B = 1; // for mapper #65f
	}
	if (REG_mapper ==6) {	// iNES Mapper #163: 南晶
		if (Addr ==0x101) {
			if (REG_R4 && !Val) REG_R5 ^= 1;
			REG_R4 = Val;
		} else if (Addr ==0x100 && Val ==6) {
			REG_PRG_mode =REG_PRG_mode &0xFE;
			REG_PRG_bank_B =12;
		} else switch((Addr >>8) &3) {
			case 2:	REG_PRG_mode |=1;
				REG_PRG_bank_A = (REG_PRG_bank_A & 0x3F) | ((Val & 3) << 6);
				REG_R0 = Val;
				break;
			case 0:	REG_PRG_mode |= 1;
				REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Val & 0x0F) << 2);
				REG_CHR_mode = (REG_CHR_mode&0xFE) | (Val>>7);
				REG_R1 = Val;
				break;
			case 3:	REG_R2 = Val;
				break;
			case 1:	REG_R3 = Val;
				break;
		}
	} 
	if (REG_mapper ==15) switch (Addr) {	// iNES Mapper #005: Nintendo MMC3
		case 0x105:	if (Val == 0xFF)
					REG_four_screen = 1;
				else {
					REG_four_screen = 0;
					switch (((Val >> 2) & 1) | ((Val >> 3) & 2)) {
						case 0: REG_mirroring = 2; break;
						case 1:	REG_mirroring = 0; break;
						case 2:	REG_mirroring = 1; break;
						case 3:	REG_mirroring = 3; break;
					}
				}
				break;
		case 0x115:	REG_PRG_bank_A = Val & ~1;
				REG_PRG_bank_B = Val | 1;
				break;
		case 0x116:	REG_PRG_bank_C = Val;
				break;
		case 0x117:	REG_PRG_bank_D = Val;
				break;
		case 0x120:	REG_CHR_bank_A = Val;
				break;
		case 0x121:	REG_CHR_bank_B = Val;
				break;
		case 0x122:	REG_CHR_bank_C = Val;
				break;
		case 0x123:	REG_CHR_bank_D = Val;
				break;
		case 0x128:	REG_CHR_bank_E = Val;
				break;
		case 0x129:	REG_CHR_bank_F = Val;
				break;
		case 0x12A:	REG_CHR_bank_G = Val;
				break;
		case 0x12B:	REG_CHR_bank_H = Val;
				break;
		case 0x203:	EMU->SetIRQ(1);
				REG_scanline2_IRQ_pending = 0;
				REG_scanline2_IRQ_line = Val;
				break;
		case 0x204:	EMU->SetIRQ(1);
				REG_scanline2_IRQ_pending = 0;
				REG_scanline2_IRQ_enabled = Val & 0x80;
				break;
	}
	// iNES Mapper #189: TXC 01-22018-400
	if ((REG_mapper ==20) && (REG_flags &2)) REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Val & 0x0F) << 2) | ((Val & 0xF0) >> 2);
	
	Sync();
}

void	MAPINT	Write67 (int Bank, int Addr, int Val) {
	_Write[Bank](Bank, Addr, Val);

	// iNES Mapper #87: Konami/Jaleco CHR-ROM switch
	if (REG_mapper ==12) REG_CHR_bank_A = (REG_CHR_bank_A & 0xE7) | ((Val & 1) << 4) | ((Val & 2) << 2);
	
	// iNES Mapper #189: TXC 01-22018-400
	if (REG_mapper ==20 && REG_flags &2) REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Val & 0x0F) << 2) | ((Val & 0xF0) >> 2);
}

void	MAPINT	Write8F (int Bank, int Addr, int Val) {
	if (REG_mapper ==1) {	// iNES Mapper #002: Nintendo UNROM/UOROM
		if (~REG_flags &1 || Bank !=0x9)
			REG_PRG_bank_A = (REG_PRG_bank_A & 0xC1) | ((Val & 0x1F) << 1);
		else
			REG_mirroring = 2 + ((Val >> 4) & 1);
	} 
	if (REG_mapper ==2) {	// iNES Mapper #003: Nintendo CNROM
		REG_CHR_bank_A = (REG_CHR_bank_A & 7) | (Val << 3);
	} 
	if (REG_mapper ==3) {	// iNES Mapper #078: Irem Holy Diver
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xF1) | ((Val & 7) << 1);
		REG_CHR_bank_A = (REG_CHR_bank_A & 0x87) | ((Val & 0xF0) >> 1);
		REG_mirroring = ((Val >> 3) & 1) ^ 1;
	} 
	if (REG_mapper ==4) {	// iNES Mapper #097: Irem TAM-S1
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xE1) | ((Val & 0x0F) << 1);
		REG_mirroring = (Val >> 6) ^ ((Val >> 6) & 2);
	} 
	if (REG_mapper ==5) {	// iNES Mapper #093: Sunsoft-2
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xF1) | ((Val & 0x70) >> 3);
		REG_can_write_CHR_RAM = Val & 1;
	} 
	if (REG_mapper ==7) switch (((Bank &7) <<2) | (Addr &3)) {	// iNES Mapper #018: Jaleco
		case 0:	REG_PRG_bank_A = (REG_PRG_bank_A & 0xF0) | (Val & 0x0F); break;
		case 1:	REG_PRG_bank_A = (REG_PRG_bank_A & 0x0F) | ((Val & 0x0F) << 4); break;
		case 2:	REG_PRG_bank_B = (REG_PRG_bank_B & 0xF0) | (Val & 0x0F); break;
		case 3:	REG_PRG_bank_B = (REG_PRG_bank_B & 0x0F) | ((Val & 0x0F) << 4); break;
		case 4:	REG_PRG_bank_C = (REG_PRG_bank_C & 0xF0) | (Val & 0x0F); break; 
		case 5:	REG_PRG_bank_C = (REG_PRG_bank_C & 0x0F) | ((Val & 0x0F) << 4); break;
		case 6:	break;
		case 7:	break;
		case 8:	REG_CHR_bank_A = (REG_CHR_bank_A & 0xF0) | (Val & 0x0F); break;
		case 9:	REG_CHR_bank_A = (REG_CHR_bank_A & 0x0F) | ((Val & 0x0F) << 4); break;
		case 10:REG_CHR_bank_B = (REG_CHR_bank_B & 0xF0) | (Val & 0x0F); break;
		case 11:REG_CHR_bank_B = (REG_CHR_bank_B & 0x0F) | ((Val & 0x0F) << 4); break;
		case 12:REG_CHR_bank_C = (REG_CHR_bank_C & 0xF0) | (Val & 0x0F); break;
		case 13:REG_CHR_bank_C = (REG_CHR_bank_C & 0x0F) | ((Val & 0x0F) << 4); break;
		case 14:REG_CHR_bank_D = (REG_CHR_bank_D & 0xF0) | (Val & 0x0F); break;
		case 15:REG_CHR_bank_D = (REG_CHR_bank_D & 0x0F) | ((Val & 0x0F) << 4); break;
		case 16:REG_CHR_bank_E = (REG_CHR_bank_E & 0xF0) | (Val & 0x0F); break;
		case 17:REG_CHR_bank_E = (REG_CHR_bank_E & 0x0F) | ((Val & 0x0F) << 4); break;
		case 18:REG_CHR_bank_F = (REG_CHR_bank_F & 0xF0) | (Val & 0x0F); break;
		case 19:REG_CHR_bank_F = (REG_CHR_bank_F & 0x0F) | ((Val & 0x0F) << 4); break;
		case 20:REG_CHR_bank_G = (REG_CHR_bank_G & 0xF0) | (Val & 0x0F); break;
		case 21:REG_CHR_bank_G = (REG_CHR_bank_G & 0x0F) | ((Val & 0x0F) << 4); break;
		case 22:REG_CHR_bank_H = (REG_CHR_bank_H & 0xF0) | (Val & 0x0F); break;
		case 23:REG_CHR_bank_H = (REG_CHR_bank_H & 0x0F) | ((Val & 0x0F) << 4); break;
		case 24:REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xFFF0) | (Val & 0x0F); break;
		case 25:REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xFF0F) | ((Val & 0x0F) << 4); break;
		case 26:REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xF0FF) | ((Val & 0x0F) << 8); break;
		case 27:REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0x0FFF) | ((Val & 0x0F) << 12); break;
		case 28:EMU->SetIRQ(1);
			REG_CPU_IRQ_value = REG_CPU_IRQ_latch; break;
		case 29:EMU->SetIRQ(1);
			REG_CPU_IRQ_control = Val & 0x0F;
			break;
		case 30:REG_mirroring = Val ^ (((Val >> 1) & 1) ^ 1); break;
		case 31:break;
	}
	if (REG_mapper ==8) {	// iNES Mapper #007: Nintendo AMROM/ANROM/AOROM; iNES Mapper #034/#241: Nintendo BNROM
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Val & 0xF) << 2);
		if (!REG_flags&1) REG_mirroring = 2 + ((Val >> 4) & 1);
	}
	if (REG_mapper ==9) {	// iNES Mapper #228: Cheetahmen II				
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xC3) | ((Addr & 0x780) >> 5);
		REG_CHR_bank_A = (REG_CHR_bank_A & 7) | ((Addr & 7) << 5) | ((Val & 3) << 3);
		REG_mirroring = (Bank >>1) & 1;
	}
	if (REG_mapper ==10) {	// iNES Mapper #011: Color Dreams
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xF3) | ((Val & 3) << 2);
		REG_CHR_bank_A = (REG_CHR_bank_A & 0x87) | ((Val & 0xF0) >> 1);
	}
	if (REG_mapper ==11) {	// iNES Mapper #066: Nintendo GNROM/MHROM
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xF3) | ((Val & 0x30) >> 2); // prg_bank_a[3:2] = cpu_data_in[5:4];
		REG_CHR_bank_A = (REG_CHR_bank_A & 0xE7) | ((Val & 3) << 3); // chr_bank_a[4:3] = cpu_data_in[1:0];
	}
	if (REG_mapper ==13) switch (Bank &7) { // iNES Mapper #090: 晶太
		case 0:	switch (Addr &3) {
				case 0: REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F); break; // $8000
				case 1: REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break; // $8001
				case 2: REG_PRG_bank_C = (REG_PRG_bank_C & 0xC0) | (Val & 0x3F); break; // $8002
				case 3: REG_PRG_bank_D = (REG_PRG_bank_D & 0xC0) | (Val & 0x3F); break; // $8003
			}
			break;
		case 1:	switch (Addr &7) {
				case 0: REG_CHR_bank_A = Val; break; // $9000
				case 1: REG_CHR_bank_B = Val; break; // $9001
				case 2: REG_CHR_bank_C = Val; break; // $9002
				case 3: REG_CHR_bank_D = Val; break; // $9003
				case 4: REG_CHR_bank_E = Val; break; // $9004
				case 5: REG_CHR_bank_F = Val; break; // $9005
				case 6: REG_CHR_bank_G = Val; break; // $9006
				case 7: REG_CHR_bank_H = Val; break; // $9007
			}
			break;
		case 4:	switch (Addr &7) {
				case 0:	if (Val &1)
						REG_scanline_IRQ_enabled = 1;
					else {
						REG_scanline_IRQ_enabled = 0;
						EMU->SetIRQ(1);
					}
					break;
				case 1:	break;
				case 2:	REG_scanline_IRQ_enabled = 0;
					EMU->SetIRQ(1);
					break;
				case 3:	REG_scanline_IRQ_enabled = 1;
					break; 
				case 4:	break;
				case 5:	REG_scanline_IRQ_latch = Val ^ REG_R0;
					REG_scanline_IRQ_reload = 1;
					break;
				case 6:	REG_R0 = Val;
					break;
				case 7:	break;
			}		
		case 5:	switch (Addr &3) {
				case 1: REG_mirroring = Val & 3; break; // $D001
			}
			break;
	}
	if (REG_mapper ==14) switch (((Bank &7) <<3) | (Addr &7)) { // iNES Mapper #065: Irem H3001
		case 0:	 REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F); break;
		case 9:	 REG_mirroring = (Val>>7)&1; break;
		case 11: REG_CPU_IRQ_control = (REG_CPU_IRQ_control & 0xFE) | ((Val>>7)&1);
			 EMU->SetIRQ(1);
			 break;
		case 12: REG_CPU_IRQ_value = (REG_R0 << 8) | REG_R1;
			 EMU->SetIRQ(1);
			 break;
		case 13: REG_R0 = Val; break;
		case 14: REG_R1 = Val; break;
		case 16: REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break;
		case 24: REG_CHR_bank_A = Val; break;
		case 25: REG_CHR_bank_B = Val; break;
		case 26: REG_CHR_bank_C = Val; break;
		case 27: REG_CHR_bank_D = Val; break;
		case 28: REG_CHR_bank_E = Val; break;
		case 29: REG_CHR_bank_F = Val; break;
		case 30: REG_CHR_bank_G = Val; break;
		case 31: REG_CHR_bank_H = Val; break;
		case 32: REG_PRG_bank_C = (REG_PRG_bank_C & 0xC0) | (Val & 0x3F); break;
	}
	if (REG_mapper ==16) {	// iNES Mapper #001: Nintendo SxROM. r0 - load register, flag0 - 16KB of WRAM (SOROM)
		if (Val &0x80) { // reset
			REG_R0 = (REG_R0 & 0xC0) | 0x20;
			REG_PRG_mode = 0;
			REG_PRG_bank_C = (REG_PRG_bank_C & 0xE0) | 0x1E;
		} else {
			REG_R0 = (REG_R0 & 0xC0) | ((Val & 1) << 5) | ((REG_R0 & 0x3E) >> 1);
			if (REG_R0 & 1)	{
				switch ((Bank >>1) &3) {
				case 0:	if ((REG_R0 & 0x18) == 0x18) {
						REG_PRG_mode = 0;
						REG_PRG_bank_C = (REG_PRG_bank_C & 0xE0) | 0x1E;
					} else if ((REG_R0 & 0x18) == 0x10) {
						REG_PRG_mode = 1;
						REG_PRG_bank_C = (REG_PRG_bank_C & 0xE0);
					} else 
						REG_PRG_mode = 7;
					if ((REG_R0 >> 5) & 1)
						REG_CHR_mode = 4;
					else
						REG_CHR_mode = 0;
					REG_mirroring = ((REG_R0 >> 1) & 3) ^ 2;
					break;
				case 1: REG_CHR_bank_A = (REG_CHR_bank_A & 0x83) | ((REG_R0 & 0x3E) << 1);
					REG_PRG_bank_A = (REG_PRG_bank_A & 0xDF) | (REG_R0 & 0x20);
					REG_PRG_bank_C = (REG_PRG_bank_C & 0xDF) | (REG_R0 & 0x20);
					break;
				case 2:	REG_CHR_bank_E = (REG_CHR_bank_E & 0x83) | ((REG_R0 & 0x3E) << 1);
					break;
				case 3:	REG_PRG_bank_A = (REG_PRG_bank_A & 0xE1) | (REG_R0 & 0x1E);
					REG_WRAM_enabled = ((REG_R0 >> 5) & 1) ^ 1;
					break;
				}
				REG_R0 = (REG_R0 & 0xC0) | 0x20;
				if (REG_flags &1) { // (flags[0]) - 16KB of WRAM
					if (REG_CHR_mode & 4)
						REG_WRAM_page = 2 | (REG_CHR_bank_A >> 6) ^ 1;
					else
						REG_WRAM_page = 2 | (REG_CHR_bank_A >> 5) ^ 1;
				}
			}
		}
	}
	if (REG_mapper ==17) switch (Bank &7) {	// iNES Mapper #009/010: Nintendo PNROM/FJROM/FKROM. flag0 - 0=MMC2, 1=MMC4
		case 2: if (!(REG_flags & 1)) // MMC2
				REG_PRG_bank_A = (REG_PRG_bank_A & 0xF0) | (Val & 0x0F);
			else // MMC4
				REG_PRG_bank_A = (REG_PRG_bank_A & 0xE1) | ((Val & 0x0F) << 1);
			break;
		case 3: REG_CHR_bank_A = (REG_CHR_bank_A & 0x83) | ((Val & 0x1F) << 2);
			break;
		case 4: REG_CHR_bank_B = (REG_CHR_bank_B & 0x83) | ((Val & 0x1F) << 2);
			break;
		case 5: REG_CHR_bank_E = (REG_CHR_bank_E & 0x83) | ((Val & 0x1F) << 2);
			break;
		case 6: REG_CHR_bank_F = (REG_CHR_bank_F & 0x83) | ((Val & 0x1F) << 2);
			break;
		case 7: REG_mirroring = Val & 1;
			break;
	}
	if (REG_mapper ==18) {	// Mapper #152
		REG_CHR_bank_A = (REG_CHR_bank_A & 0x87) | ((Val & 0x0F) << 3);
		REG_PRG_bank_A = (REG_PRG_bank_A & 0xF1) | ((Val & 0x70) >> 3);
		REG_mirroring = 2 | (Val>>7);
	}
	if (REG_mapper ==19) switch (Bank &7) {	// Mapper #73 - VRC3
		case 0: REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xFFF0) | (Val & 0x0F);
			break;
		case 1: REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xFF0F) | ((Val & 0x0F) << 4);
			break;
		case 2: REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xF0FF) | ((Val & 0x0F) << 8);
			break;
		case 3: REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0x0FFF) | ((Val & 0x0F) << 12);
			break; 
		case 4: EMU->SetIRQ(1);
			REG_CPU_IRQ_control = (REG_CPU_IRQ_control & 0xF8) | (Val & 7);
			if (REG_CPU_IRQ_control &2) REG_CPU_IRQ_value = REG_CPU_IRQ_latch;
			break;
		case 5: EMU->SetIRQ(1);
			REG_CPU_IRQ_control = (REG_CPU_IRQ_control & 0xFD) | (REG_CPU_IRQ_control & 1) << 1;
			break;
		case 7: REG_PRG_bank_A = (REG_PRG_bank_A & 0xF1) | ((Val & 7) << 1);
			break;
	}
	if (REG_mapper ==20) switch ((Bank &6) | (Addr &1)) { // iNES Mapper #004/118/189: TxROM/TxSROM/TXC 01-22018-400. r0[2:0] - internal register, flag0 - TxSROM, flag1 - mapper #189
		case 0: REG_R0 = (REG_R0 & 0xF8) | (Val & 7);
			if (!(REG_flags & 2)) REG_PRG_mode = (Val &0x40)? 5: 4;
			REG_CHR_mode =(Val &0x80)? 3: 2;
			break;
		case 1: switch (REG_R0 &7)  {
				case 0:	REG_CHR_bank_A = Val; break;
				case 1:	REG_CHR_bank_C = Val; break;
				case 2:	REG_CHR_bank_E = Val; break;
				case 3:	REG_CHR_bank_F = Val; break;
				case 4:	REG_CHR_bank_G = Val; break;
				case 5:	REG_CHR_bank_H = Val; break;
				case 6: if (!(REG_flags & 2)) REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F);
					break;
				case 7:	if (!(REG_flags & 2)) REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F);
					break;
			}
			break;
		case 2: REG_mirroring = Val & 1;
			break;
		case 3: break;
		case 4: REG_scanline_IRQ_latch = Val;
			break;
		case 5: REG_scanline_IRQ_reload = 1;
			break;
		case 6: REG_scanline_IRQ_enabled = 0;
			EMU->SetIRQ(1);
			break;
		case 7: REG_scanline_IRQ_enabled = 1;
			break;
	}
	if (REG_mapper ==21) switch ((Bank >>1) &3) {	// iNES Mapper #112: NTDEC rewired N108 clone. r0[2:0] - internal register
		case 0: REG_R0 = (REG_R0 & 0xF8) | (Val & 7); break;
		case 1: switch (REG_R0 &7) {
				case 0:	REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F); break;
				case 1:	REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break;
				case 2:	REG_CHR_bank_A = Val; break;
				case 3:	REG_CHR_bank_C = Val; break;
				case 4:	REG_CHR_bank_E = Val; break;
				case 5:	REG_CHR_bank_F = Val; break;
				case 6:	REG_CHR_bank_G = Val; break;
				case 7:	REG_CHR_bank_H = Val; break;
			}
			break;
		case 3: REG_mirroring = Val & 1; break;
	}
	if (REG_mapper ==22) switch (((Bank <<1) &0xC) | (Addr &0x3)) {	// iNES Mapper #033/048: Taito. // flag0=0 - #33, flag0=1 - #48
		case 0:	 REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F);
			 if (!(REG_flags & 1)) REG_mirroring = (Val >> 6) & 1;
			 break;
		case 1:  REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break;
		case 2:  REG_CHR_bank_A = Val << 1; break;
		case 3:  REG_CHR_bank_C = Val << 1; break;
		case 4:  REG_CHR_bank_E = Val; break;
		case 5:  REG_CHR_bank_F = Val; break;
		case 6:  REG_CHR_bank_G = Val; break;
		case 7:  REG_CHR_bank_H = Val; break;
		case 12: if (REG_flags &1) REG_mirroring = (Val >> 6) & 1;
		case 8:  REG_scanline_IRQ_latch = Val; break;
		case 9:  REG_scanline_IRQ_reload = 1; break;
		case 10: REG_scanline_IRQ_enabled = 1; break;
		case 11: REG_scanline_IRQ_enabled = 0;
			 EMU->SetIRQ(1);
			 break;
	}	
	if (REG_mapper ==24) {	// iNES Mapper #021/022/023/025: Konami VRC2/4.
		/*	flag0 - switches A0 and A1 lines. 0=A0,A1 like VRC2b (mapper #23), 1=A1,A0 like VRC2a(#22), VRC2c(#25)
			flag1 - divides CHR bank select by two (mapper #22, VRC2a) */
		Addr |=Bank <<12;
		uint8_t vrc_2b_hi = ((Addr >> 1) & 1) | ((Addr >> 3) & 1) | ((Addr >> 5) & 1) | ((Addr >> 7) & 1);
		uint8_t vrc_2b_low = (Addr & 1) | ((Addr >> 2) & 1) | ((Addr >> 4) & 1) | ((Addr >> 6) & 1);
		switch (((Addr >> 10) & 0x1C) | (((REG_flags & 1) ? vrc_2b_low : vrc_2b_hi) << 1) | ((REG_flags & 1) ? vrc_2b_hi : vrc_2b_low)) {
			case 0: case 1: case 2: case 3:
				REG_PRG_bank_A = (REG_PRG_bank_A & 0xE0) | (Val & 0x1F);
				break;
			case 4: case 5:
				if (Val != 0xFF) REG_mirroring = Val & 3; // $FF check: prevent Wai Wai World (VRC2) from using VRC4's one-screen mirroring
				break;
			case 6:	case 7:
				REG_PRG_mode = (REG_PRG_mode & 0xFE) | ((Val >> 1) & 1);
				break;
			case 8: case 9: case 10:case 11:
				REG_PRG_bank_B = (REG_PRG_bank_B & 0xE0) | (Val & 0x1F);
				break;
		}
		if (!(REG_flags & 2)) switch (((Addr >> 10) & 0x1C) | (((REG_flags & 1) ? vrc_2b_low : vrc_2b_hi) << 1) | ((REG_flags & 1) ? vrc_2b_hi : vrc_2b_low))  {
			case 12: REG_CHR_bank_A = (REG_CHR_bank_A & 0xF0) | (Val & 0x0F); break;
			case 13: REG_CHR_bank_A = (REG_CHR_bank_A & 0x0F) | ((Val & 0x0F) << 4); break;
			case 14: REG_CHR_bank_B = (REG_CHR_bank_B & 0xF0) | (Val & 0x0F); break;
			case 15: REG_CHR_bank_B = (REG_CHR_bank_B & 0x0F) | ((Val & 0x0F) << 4); break;
			case 16: REG_CHR_bank_C = (REG_CHR_bank_C & 0xF0) | (Val & 0x0F); break;
			case 17: REG_CHR_bank_C = (REG_CHR_bank_C & 0x0F) | ((Val & 0x0F) << 4); break;
			case 18: REG_CHR_bank_D = (REG_CHR_bank_D & 0xF0) | (Val & 0x0F); break;
			case 19: REG_CHR_bank_D = (REG_CHR_bank_D & 0x0F) | ((Val & 0x0F) << 4); break;
			case 20: REG_CHR_bank_E = (REG_CHR_bank_E & 0xF0) | (Val & 0x0F); break;
			case 21: REG_CHR_bank_E = (REG_CHR_bank_E & 0x0F) | ((Val & 0x0F) << 4); break;
			case 22: REG_CHR_bank_F = (REG_CHR_bank_F & 0xF0) | (Val & 0x0F); break;
			case 23: REG_CHR_bank_F = (REG_CHR_bank_F & 0x0F) | ((Val & 0x0F) << 4); break;
			case 24: REG_CHR_bank_G = (REG_CHR_bank_G & 0xF0) | (Val & 0x0F); break;
			case 25: REG_CHR_bank_G = (REG_CHR_bank_G & 0x0F) | ((Val & 0x0F) << 4); break;
			case 26: REG_CHR_bank_H = (REG_CHR_bank_H & 0xF0) | (Val & 0x0F); break;
			case 27: REG_CHR_bank_H = (REG_CHR_bank_H & 0x0F) | ((Val & 0x0F) << 4); break;
		}
		else switch (((Addr >> 10) & 0x1C) | (((REG_flags & 1) ? vrc_2b_low : vrc_2b_hi) << 1) | ((REG_flags & 1) ? vrc_2b_hi : vrc_2b_low)) {
			case 12: REG_CHR_bank_A = (REG_CHR_bank_A & 0x78) | ((Val & 0x0E) >> 1); break;
			case 13: REG_CHR_bank_A = (REG_CHR_bank_A & 0x07) | ((Val & 0x0F) << 3); break;
			case 14: REG_CHR_bank_B = (REG_CHR_bank_B & 0x78) | ((Val & 0x0E) >> 1); break;
			case 15: REG_CHR_bank_B = (REG_CHR_bank_B & 0x07) | ((Val & 0x0F) << 3); break;
			case 16: REG_CHR_bank_C = (REG_CHR_bank_C & 0x78) | ((Val & 0x0E) >> 1); break;
			case 17: REG_CHR_bank_C = (REG_CHR_bank_C & 0x07) | ((Val & 0x0F) << 3); break;
			case 18: REG_CHR_bank_D = (REG_CHR_bank_D & 0x78) | ((Val & 0x0E) >> 1); break;
			case 19: REG_CHR_bank_D = (REG_CHR_bank_D & 0x07) | ((Val & 0x0F) << 3); break;
			case 20: REG_CHR_bank_E = (REG_CHR_bank_E & 0x78) | ((Val & 0x0E) >> 1); break;
			case 21: REG_CHR_bank_E = (REG_CHR_bank_E & 0x07) | ((Val & 0x0F) << 3); break;
			case 22: REG_CHR_bank_F = (REG_CHR_bank_F & 0x78) | ((Val & 0x0E) >> 1); break;
			case 23: REG_CHR_bank_F = (REG_CHR_bank_F & 0x07) | ((Val & 0x0F) << 3); break;
			case 24: REG_CHR_bank_G = (REG_CHR_bank_G & 0x78) | ((Val & 0x0E) >> 1); break;
			case 25: REG_CHR_bank_G = (REG_CHR_bank_G & 0x07) | ((Val & 0x0F) << 3); break;
			case 26: REG_CHR_bank_H = (REG_CHR_bank_H & 0x78) | ((Val & 0x0E) >> 1); break;
			case 27: REG_CHR_bank_H = (REG_CHR_bank_H & 0x07) | ((Val & 0x0F) << 3); break;
		}
		if (((Addr >> 12) & 7) == 7) switch ((((REG_flags & 1) ? vrc_2b_low : vrc_2b_hi) << 1) | ((REG_flags & 1) ? vrc_2b_hi : vrc_2b_low)) {
			case 0:	REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0xF0) | (Val & 0x0F); break;
			case 1:	REG_CPU_IRQ_latch = (REG_CPU_IRQ_latch & 0x0F) | ((Val & 0x0F) << 4); break;
			case 2: EMU->SetIRQ(1);
				REG_CPU_IRQ_control = (REG_CPU_IRQ_control & 0xF8) | (Val & 7);
				if (REG_CPU_IRQ_control & 2) {
					REG_VRC4_IRQ_prescaler_counter = 0;
					REG_VRC4_IRQ_prescaler = 0;
					REG_CPU_IRQ_value = REG_CPU_IRQ_latch;
				}
				break;
			case 3: EMU->SetIRQ(1);
				REG_CPU_IRQ_control = (REG_CPU_IRQ_control & 0xFD) | (REG_CPU_IRQ_control & 1) << 1;
				break;
		}
	}
	if (REG_mapper ==25) {	// iNES Mapper #069: Sunsoft FME-7. r0 - command register
		if (((Bank >>1) &3) ==0) REG_R0 = (REG_R0 & 0xF0) | (Val & 0x0F);
		if (((Bank >>1) &3) ==1) switch (REG_R0 & 0x0F) {
			case 0:  REG_CHR_bank_A = Val; break;
			case 1:  REG_CHR_bank_B = Val; break;
			case 2:  REG_CHR_bank_C = Val; break;
			case 3:  REG_CHR_bank_D = Val; break;
			case 4:  REG_CHR_bank_E = Val; break;
			case 5:  REG_CHR_bank_F = Val; break;
			case 6:  REG_CHR_bank_G = Val; break;
			case 7:  REG_CHR_bank_H = Val; break;
			case 8:  REG_WRAM_enabled = (Val >> 7) & 1;
				 REG_map_ROM_on_6000 = ((Val >> 6) & 1) ^ 1;
				 REG_PRG_bank_6000 = Val & 0x3F;
				 break;
			case 9:  REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F); break;
			case 10: REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break;
			case 11: REG_PRG_bank_C = (REG_PRG_bank_C & 0xC0) | (Val & 0x3F); break;
			case 12: REG_mirroring = Val & 3; break;
			case 13: REG_CPU_IRQ_control = ((Val >> 6) & 1) | (Val & 1);
				 EMU->SetIRQ(1);
				 break;
			case 14: REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xFF00) | Val; break;
			case 15: REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0x00FF) | (Val << 8); break;
		}
	}
	if (REG_mapper ==26) switch (Bank &3) {	// iNES Mapper #032: Irem G-101
		case 0: REG_PRG_bank_A = (REG_PRG_bank_A & 0xC0) | (Val & 0x3F); break;
		case 1: REG_PRG_mode = (REG_PRG_mode & 6) | ((Val >> 1) & 1);
			REG_mirroring = Val & 1;
			break;
		case 2: REG_PRG_bank_B = (REG_PRG_bank_B & 0xC0) | (Val & 0x3F); break;
		case 3: switch (Addr &7) {
				case 0:	REG_CHR_bank_A = Val; break;
				case 1:	REG_CHR_bank_B = Val; break;
				case 2:	REG_CHR_bank_C = Val; break;
				case 3:	REG_CHR_bank_D = Val; break;
				case 4: REG_CHR_bank_E = Val; break;
				case 5: REG_CHR_bank_F = Val; break;
				case 6: REG_CHR_bank_G = Val; break;
				case 7: REG_CHR_bank_H = Val; break;
			}
			break;
	}
	if (REG_mapper ==31) {	// Temp/Test
		REG_PRG_bank_6000 = ((Val>>1)&0xF) + 4;
		REG_map_ROM_on_6000 = 1;
	}
	
	Sync();
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0x0; i<=0xF; i++) {
		_Read[i] = EMU->GetCPUReadHandler(i);
		_Write[i] = EMU->GetCPUWriteHandler(i);
	}
	EMU->SetCPUReadHandler(0x5, Read5);
	EMU->SetCPUWriteHandler(0x4, Write4);
	EMU->SetCPUWriteHandler(0x5, Write5);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUWriteHandler(i, Write67);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write8F);
	
	REG_WRAM_enabled = 0;
	REG_WRAM_page = 0;
	REG_can_write_CHR_RAM = 0;
	REG_map_ROM_on_6000 = 0;
	REG_flags = 0;
	REG_mapper = 0;
	REG_can_write_PRG = 0;
	REG_mirroring = 0;
	REG_four_screen = 0;
	REG_lockout = 0;
	REG_PRG_base = 0;
	REG_PRG_mask = 0xF8; // 11111000, 128KB
	REG_PRG_mode = 0;
	REG_PRG_bank_6000 = 0;
	REG_PRG_bank_A = 0;
	REG_PRG_bank_B = 1; 
	REG_PRG_bank_C = 0xFE;
	REG_PRG_bank_D = 0xFF;
	REG_CHR_mask = 0;
	REG_CHR_mode = 0;
	REG_CHR_bank_A = 0;
	REG_CHR_bank_B = 1;
	REG_CHR_bank_C = 2;
	REG_CHR_bank_D = 3;
	REG_CHR_bank_E = 4;
	REG_CHR_bank_F = 5;
	REG_CHR_bank_G = 6;
	REG_CHR_bank_H = 7;
	REG_scanline_IRQ_enabled = 0;
	REG_scanline_IRQ_counter = 0;
	REG_scanline_IRQ_latch = 0;
	REG_scanline_IRQ_reload = 0;
	REG_scanline2_IRQ_enabled = 0;
	REG_scanline2_IRQ_line = 0;
	REG_CPU_IRQ_value = 0;
	REG_CPU_IRQ_control = 0;
	REG_CPU_IRQ_latch = 0;
	REG_VRC4_IRQ_prescaler = 0;
	REG_VRC4_IRQ_prescaler_counter = 0;
	REG_R0 = 0;
	REG_R1 = 0;
	REG_R2 = 0;
	REG_R3 = 0;
	REG_R4 = 0;
	REG_R5 = 0;
	Sync();
}

void	MAPINT	CPUCycle (void) {
	// Mapper #24 - VRC4
	if (REG_mapper == 24 && (REG_CPU_IRQ_control & 2)) {
		if (REG_CPU_IRQ_control & 4) {
			REG_CPU_IRQ_value++;
			if ((REG_CPU_IRQ_value & 0xFF) == 0) {
				EMU->SetIRQ(0);
				REG_CPU_IRQ_value = REG_CPU_IRQ_latch;
			}
		} else {
			REG_VRC4_IRQ_prescaler++;
			if ((!(REG_VRC4_IRQ_prescaler_counter&2) && REG_VRC4_IRQ_prescaler == 114) || ((REG_VRC4_IRQ_prescaler_counter&2) && REG_VRC4_IRQ_prescaler == 113)) {
				REG_CPU_IRQ_value++;
				REG_VRC4_IRQ_prescaler = 0;
				REG_VRC4_IRQ_prescaler_counter++;
				if (REG_VRC4_IRQ_prescaler_counter == 3) REG_VRC4_IRQ_prescaler_counter = 0;
				if ((REG_CPU_IRQ_value & 0xFF) == 0) {
					EMU->SetIRQ(0);
					REG_CPU_IRQ_value = REG_CPU_IRQ_latch;
				}
			}
		}
	}
	// Mapper #73 - VRC3
	if (REG_mapper == 19 && (REG_CPU_IRQ_control & 2)) {
		if (REG_CPU_IRQ_control & 4) {  // 8-bit mode
			REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xFF00) | ((REG_CPU_IRQ_value+1) & 0xFF);
			if ((REG_CPU_IRQ_value & 0xFF) == 0) {
				EMU->SetIRQ(0);
				REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xFF00) | (REG_CPU_IRQ_latch & 0xFF);
			}
		} else {
			REG_CPU_IRQ_value++;
			if ((REG_CPU_IRQ_value & 0xFFFF) == 0) {
				EMU->SetIRQ(0);
				REG_CPU_IRQ_value = REG_CPU_IRQ_latch;
			}
		}
	}
	// Mapper #69 - Sunsoft FME-7
	if (REG_mapper == 25) {
		if (((REG_CPU_IRQ_value & 0xFFFFF) == 0) && (REG_CPU_IRQ_control&1)) EMU->SetIRQ(0);
		REG_CPU_IRQ_value--;
	}
	// Mapper #18
	if (REG_mapper == 7 && REG_CPU_IRQ_control & 1) {
			if (REG_CPU_IRQ_control & 8) {
				if ((REG_CPU_IRQ_value & 0x000F) == 0) EMU->SetIRQ(0); 
				REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xFFF0) | ((REG_CPU_IRQ_value - 1) & 0x000F); 
			} else if (REG_CPU_IRQ_control & 4) {
				if ((REG_CPU_IRQ_value & 0x00FF) == 0) EMU->SetIRQ(0); 
				REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xFF00) | ((REG_CPU_IRQ_value - 1) & 0x00FF);
			} else if (REG_CPU_IRQ_control & 2) {
				if ((REG_CPU_IRQ_value & 0x0FFF) == 0) EMU->SetIRQ(0);
				REG_CPU_IRQ_value = (REG_CPU_IRQ_value & 0xF000) | ((REG_CPU_IRQ_value - 1) & 0x0FFF);
			} else {
				if ((REG_CPU_IRQ_value & 0xFFFF) == 0) EMU->SetIRQ(0);
				REG_CPU_IRQ_value = ((REG_CPU_IRQ_value - 1) & 0xFFFF);
			}
	}
	// Mapper #65 - Irem's H3001
	if (REG_mapper == 14 && REG_CPU_IRQ_control & 1 && (REG_CPU_IRQ_value & 0xFFFF) > 0) {
		REG_CPU_IRQ_value--;
		if (!REG_CPU_IRQ_value) EMU->SetIRQ(0);
	}
}

void	MAPINT	PPUCycle (int Addr, int Scanline, int Cycle, int IsRendering) {
	LastPPUAddr =Addr;
	LastPPUScanline =Scanline;
	LastPPUCycle =Cycle;
	LastPPUIsRendering =IsRendering;
	
	if (IsRendering && Cycle ==260) { // Scanline counter
		if (REG_scanline_IRQ_reload || !REG_scanline_IRQ_counter) {
			REG_scanline_IRQ_counter = REG_scanline_IRQ_latch;
			REG_scanline_IRQ_reload = 0;
		} else
			REG_scanline_IRQ_counter--;
		if (!REG_scanline_IRQ_counter && REG_scanline_IRQ_enabled) EMU->SetIRQ(0);
	
		// for MMC5
		if (REG_scanline2_IRQ_line == Scanline+1 && REG_scanline2_IRQ_enabled) {
			EMU->SetIRQ(0);
			REG_scanline2_IRQ_pending = 1;
		}
	
		// for mapper #163
		if (REG_mapper == 6) {
			if (Scanline == 239) {
				ppu_mapper163_latch = 0;
				SyncCHR();
			} else if (Scanline == 127) {
				ppu_mapper163_latch = 1;
				SyncCHR();
			}
		}
	}
	// TKSROM/TLSROM
	if ((REG_mapper ==20) && (REG_flags & 1)) {
		if (TKSMIR[(Addr & 0x1FFF) >> 10])
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	}
	// Mapper #9 and #10 - MMC2 and MMC4
	if (REG_mapper == 17) {
		if ((Addr >> 4) == 0xFD) {
			ppu_latch0 = 0;
			SyncCHR();
		}
		if ((Addr >> 4) == 0xFE) {
			ppu_latch0 = 1;
			SyncCHR();
		}
		if ((Addr >> 4) == 0x1FD) {
			ppu_latch1 = 0;
			SyncCHR();
		}
		if ((Addr >> 4) == 0x1FE) {
			ppu_latch1 = 1;
			SyncCHR();
		}
	}	
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, REG_WRAM_enabled);
	SAVELOAD_BYTE(mode, offset, data, REG_map_ROM_on_6000);
	SAVELOAD_BYTE(mode, offset, data, REG_WRAM_page);
	SAVELOAD_BYTE(mode, offset, data, REG_can_write_CHR_RAM);
	SAVELOAD_BYTE(mode, offset, data, REG_can_write_PRG);
	SAVELOAD_BYTE(mode, offset, data, REG_flags);
	SAVELOAD_BYTE(mode, offset, data, REG_mapper);
	SAVELOAD_BYTE(mode, offset, data, REG_mirroring);
	SAVELOAD_BYTE(mode, offset, data, REG_four_screen);
	SAVELOAD_BYTE(mode, offset, data, REG_lockout);
	SAVELOAD_WORD(mode, offset, data, REG_PRG_base);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_mask);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_mode);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_bank_6000);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_bank_A);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_bank_B);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_bank_C);
	SAVELOAD_BYTE(mode, offset, data, REG_PRG_bank_D);
	SAVELOAD_LONG(mode, offset, data, REG_PRG_bank_6000_mapped);
	SAVELOAD_LONG(mode, offset, data, REG_PRG_bank_A_mapped);
	SAVELOAD_LONG(mode, offset, data, REG_PRG_bank_B_mapped);
	SAVELOAD_LONG(mode, offset, data, REG_PRG_bank_C_mapped);
	SAVELOAD_LONG(mode, offset, data, REG_PRG_bank_D_mapped);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_mask);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_mode);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_A);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_B);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_C);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_D);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_E);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_F);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_G);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_H);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_A_alt);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_B_alt);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_C_alt);
	SAVELOAD_BYTE(mode, offset, data, REG_CHR_bank_D_alt);
	SAVELOAD_BYTE(mode, offset, data, ppu_latch0);
	SAVELOAD_BYTE(mode, offset, data, ppu_latch1);
	SAVELOAD_BYTE(mode, offset, data, ppu_mapper163_latch);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, TKSMIR[i]);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline_IRQ_enabled);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline_IRQ_counter);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline_IRQ_latch);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline_IRQ_reload);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline2_IRQ_enabled);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline2_IRQ_line);
	SAVELOAD_BYTE(mode, offset, data, REG_scanline2_IRQ_pending);
	SAVELOAD_WORD(mode, offset, data, REG_CPU_IRQ_value);
	SAVELOAD_BYTE(mode, offset, data, REG_CPU_IRQ_control);
	SAVELOAD_WORD(mode, offset, data, REG_CPU_IRQ_latch);
	SAVELOAD_BYTE(mode, offset, data, REG_VRC4_IRQ_prescaler);
	SAVELOAD_BYTE(mode, offset, data, REG_VRC4_IRQ_prescaler_counter);
	SAVELOAD_BYTE(mode, offset, data, REG_R0);
	SAVELOAD_BYTE(mode, offset, data, REG_R1);
	SAVELOAD_BYTE(mode, offset, data, REG_R2);
	SAVELOAD_BYTE(mode, offset, data, REG_R3);
	SAVELOAD_BYTE(mode, offset, data, REG_R4);
	SAVELOAD_BYTE(mode, offset, data, REG_R5);
	SAVELOAD_BYTE(mode, offset, data, REG_mul1);
	SAVELOAD_BYTE(mode, offset, data, REG_mul2);	
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =342;
} // namespace

MapperInfo MapperInfo_342 ={
	&MapperNum,
	_T("COOLGIRL"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	CPUCycle,
	PPUCycle,
	SaveLoad,
	NULL,
	NULL
};