#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_SerialDevice.h"
#include	"..\..\Hardware\h_Latch.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_MMC2.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_MMC4.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\h_VRC6.h"
#include	"..\..\Hardware\h_VRC7.h"
#include	"..\..\Hardware\h_FME7.h"

static const uint16_t lut509[512] ={ /* Strange look-up table, used only by Legendary Games of NES 509-in-1 */
   7,   8,   9,  10,  11,
  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,   0,   1,  73,  74,  75,  76,  77,  78,  79,  80,  81,
  82,  83,  84,  85,  86,  87,  88,  89,  90,   4,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,   2,   3, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,   5, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 256,   6, 257, 258,
 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294,
 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330,
 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367,
 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403,
 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439,
 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475,
 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 512, 513, 514,
 515, 516, 517
};

class SerialLatch: public SerialDevice {
	uint8_t  command;
	uint8_t* rom;
public:
	         SerialLatch(uint8_t*);
	void     setPins(bool, bool, bool);
};
SerialLatch::SerialLatch(uint8_t* _rom):
	rom(_rom) {
}
void SerialLatch::setPins(bool select, bool newClock, bool newData) {
	if (select)
		state =0;
	else
	if (!clock && newClock) {
		if (state <8) {
			command =command <<1 | newData*1;
			if (++state ==8 && (command &0xF0) !=0x50 && (command &0xF0) !=0xA0) state =0;
		} else {
			int mask =1 <<(15 -state);
			int address =command &0x0F;
			unsigned int lutAddress =rom[0] | rom[1] | rom[2] <<8;
			if ((command &0xF0) ==0xA0) {
				rom[address] =rom[address] &~mask | newData*mask;
				/* The "write" command also returns the content of a strange lookup table */
				output =!!(lut509[lutAddress &0x1FF] >>(address &1? 0: 8) &mask);
			} else
			if ((command &0xF0) ==0x50)
				output =!!(rom[address] &mask);
			
			if (++state ==16) state =0;
		}
	}
	clock =newClock;
}

namespace {
uint8_t		scratchData[16];
uint8_t		vrc1Reg[8];
SerialLatch	scratchROM(scratchData);
uint16_t	reg[8], extra;
int		prgAND;
int		prgOR;
bool		pa09;
bool		pa13;

void		dummySync(void) { }
FSync		mapperSync;
int		(MAPINT *GenSound)(int) =NULL;

void	(MAPINT	*mapperCPUCycle) (void);
void	(MAPINT	*mapperPPUCycle) (int, int, int, int);
FCPURead	cartRead;
FCPUWrite	cartWrite;
FPPURead	dummyPPU;

void	sync (void) {
	if (ROM->INES2_SubMapper ==1)
		prgOR =extra  <<9 &0x2000 | reg[7] >>3 &0x1FE0 | reg[7] <<4 &0x0010;
	else
		prgOR =reg[6] <<1 &0x2000 | reg[7] >>3 &0x1FE0 | reg[7] <<4 &0x0010;
	EMU->SetPRG_RAM8(0x6, 0);
	mapperSync();
}

void	syncMMC1 (void) {
	prgOR =prgOR >>1 | reg[7] &0x0006;
	MMC1::syncPRG(prgAND, prgOR &~prgAND);
	MMC1::syncCHR(0x1F, 0x00);
	MMC1::syncMirror();
}
void	syncSUROM (void) {
	prgOR >>=1;
	MMC1::syncPRG(0x0F, MMC1::getCHRBank(0) &0x10 | prgOR &~0x1F);
	MMC1::syncCHR(0x01, 0x00);
	MMC1::syncMirror();
}
void	syncMMC2 (void) {
	MMC2::syncPRG(prgAND, prgOR &~prgAND);
	MMC2::syncCHR(0x1F, 0x00);
	MMC2::syncMirror();
}
void	syncMMC3 (void) {
	prgOR |=extra &1? 12: 0;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(reg[7] &0x10? 0xFF: 0x7F, 0x00);
	MMC3::syncMirror();
}
void	syncMMC3_OneScreenMirroring (void) {
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(0x7F, 0x00);
	switch(MMC3::mirroring &3) {
		case 0: EMU->Mirror_V(); break;
		case 1: EMU->Mirror_H(); break;
		case 2: MMC3::syncMirrorA17(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}
void	syncMMC4 (void) {
	prgOR >>=1;
	MMC4::syncPRG(prgAND, prgOR &~prgAND);
	MMC4::syncCHR(0x1F, 0x00);
	MMC4::syncMirror();
}
void	syncVRC1 (void) {
	EMU->SetPRG_ROM8(0x8, vrc1Reg[0] &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM8(0xA, vrc1Reg[2] &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM8(0xC, vrc1Reg[4] &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM8(0xE, 0xFF       &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM4(0, vrc1Reg[6] &0x0F | vrc1Reg[1] <<3 &0x10);
	EMU->SetCHR_RAM4(4, vrc1Reg[7] &0x0F | vrc1Reg[1] <<2 &0x10);
	if (vrc1Reg[1] &1)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}
void	syncVRC24 (void) {
	VRC24::syncPRG(prgAND, prgOR &~prgAND);
	VRC24::syncCHR(0xFF, 0x00);
	VRC24::syncMirror();
}
void	syncVRC3 (void) {
	prgOR >>=1;
	EMU->SetPRG_ROM16(0x8, reg[3] &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM16(0xC,         prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[7] &0x4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncVRC6 (void) {
	VRC6::syncPRG(prgAND, prgOR &~prgAND);
	VRC6::syncCHR_ROM(0xFF, 0x00);
	VRC6::syncMirror(0xFF, 0x00);
}
void	syncVRC7 (void) {
	VRC7::syncPRG(prgAND, prgOR &~prgAND);
	VRC7::syncCHR_RAM(0xFF, 0x00);
	VRC7::syncMirror();
}
void	syncUNROM (void) {
	if (Latch::addr ==0xA000 && Latch::data ==0x00 && (reg[7] &0xFF) ==0x9E) Latch::data =0x06; // ??? Needed to get #282 "Portopia Serial Murder Case" running
	prgOR >>=1;
	EMU->SetPRG_ROM16(0x8, Latch::data &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM16(0xC,              prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[7] &0x4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncIF12(void) {
	prgOR >>=1;
	EMU->SetPRG_ROM16(0x8, reg[2] &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM16(0xC,         prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, reg[0] >>1 &0xF);
	if (reg[0] &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncANROM (void) {
	prgOR >>=2;
	EMU->SetPRG_ROM32(0x8, Latch::data &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::data &0x10)
		EMU->Mirror_S1();
	else
		EMU->Mirror_S0();
}
void	syncBNROM (void) {
	prgOR >>=2;
	EMU->SetPRG_ROM32(0x8, Latch::data &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[7] &0x4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncCNROM (void) {
	if (reg[7] &0x2) {
		prgOR &=~0xF;
		EMU->SetPRG_ROM16(0x8, reg[2] <<1 &0xE | reg[7] &0x1 | prgOR >>1 &~0xF);
		EMU->SetPRG_ROM16(0xC, reg[2] <<1 &0xE | reg[7] &0x1 | prgOR >>1 &~0xF);
		EMU->SetCHR_RAM8(0x0, reg[0] &3);
	} else {
		EMU->SetPRG_ROM32(0x8, reg[2] &0x7 | prgOR >>2);
		EMU->SetCHR_RAM8(0x0, reg[0] &15);
	}
	if (reg[7] &0x8) {
		if (reg[7] &0x4)
			EMU->Mirror_H();
		else
/*		if (reg[1] &0x10)
			EMU->Mirror_H();
		else*/
			EMU->Mirror_V();
	} else {
		if (reg[1] &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	}
}
void	syncGNROM (void) {
	prgOR  =prgOR >>2 | (reg[7] &4? 2: 0);
	EMU->SetPRG_ROM32(0x8, Latch::data >>4 &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, Latch::data &15);
	if (reg[7] &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncColorDreams (void) {
	prgOR  =prgOR >>2 | (reg[7] &4? 2: 0);
	EMU->SetPRG_ROM32(0x8, Latch::data &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, Latch::data >>4 &15);
	if (reg[7] &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncNanjing (void) {
	EMU->SetPRG_ROM32(0x8, reg[2] <<4 &0x30 | reg[0] &0x0F | (reg[3] &4? 0x00: 0x03) | prgOR >>2);
	EMU->SetCHR_RAM8(0x0, 0);	
	if (reg[7] &0x4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncSMB2J(void) {
	prgOR |=reg[7] &8;			
	EMU->SetPRG_ROM8(0x8, 4         | prgOR);
	EMU->SetPRG_ROM8(0xA, 5         | prgOR);
	EMU->SetPRG_ROM8(0xC, reg[2] &7 | prgOR);
	EMU->SetPRG_ROM8(0xE, 7         | prgOR);
	EMU->SetCHR_RAM8(0x0, 0);	
	if (reg[7] &0x4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
void	syncFME7(void) {
	FME7::syncPRG(prgAND, prgOR &~prgAND);
	FME7::syncCHR(reg[7] &0x08? 0xFF: 0x7F, 0x00);
	FME7::syncMirror();
}

void	MAPINT	writeCNROM_CHR (int bank, int addr, int val) {
	reg[0] =val;
	sync();
}
void	MAPINT	writeCNROM_Mirroring (int bank, int addr, int val) {
	reg[1] =val;
	sync();
}
void	MAPINT	writeCNROM_PRG (int bank, int addr, int val) {
	reg[2] =val;
	sync();
}
void	MAPINT	writeIF12_CHR (int bank, int addr, int val) {
	reg[0] =val;
	sync();
}
void	MAPINT	writeIF12_PRG (int bank, int addr, int val) {
	reg[2] =val;
	sync();
}
void	MAPINT	writeSMB2J_IRQ (int bank, int addr, int val) {
	reg[0] =!!(bank &2);
	sync();
}
void	MAPINT	writeSMB2J_PRG (int bank, int addr, int val) {
	reg[2] =val;
	sync();
}
void	MAPINT	writeVRC1 (int bank, int addr, int val) {
	vrc1Reg[bank &7] =val;
	sync();
}
void	MAPINT	writeVRC3 (int bank, int addr, int val) {
	int shift;
	switch(bank) {
		case 0x8: case 0x9: case 0xA: case 0xB:
			val &=0xF;
			shift =bank <<2 &0xC;
			reg[1] =reg[1] &~(0xF <<shift) | val <<shift;
			break;
		case 0xC:
			reg[2] =val;
			if (reg[2] &2) reg[0] =reg[1];
			EMU->SetIRQ(1);
			break;
		case 0xD:
			reg[2] =reg[2] &~2 | reg[2] <<1 &1;
			EMU->SetIRQ(1);
			break;
		case 0xF:
			reg[3] =val;
			sync();
			break;
	}
}

void	MAPINT	cpuCycle_SMB2J (void) {
	if (reg[0]) {
		EMU->SetIRQ(++reg[1] &0x1000? 0: 1);
	} else {
		EMU->SetIRQ(1);
		reg[1] =0;
	}
}
void	MAPINT	cpuCycle_VRC3 (void) {
	if (reg[2] &2) {
		int mask =reg[2] &4? 0xFF: 0xFFFF;
		if ((reg[0] &mask) ==mask) {
			reg[0] =reg[1];
			EMU->SetIRQ(0);
		} else
			++reg[0];
	}
}

void	checkCHRSplit (int bank, int addr) { // Nanjing
	// During rising edge of PPU A13, PPU A9 is latched.
	bool pa13new =!!(bank &8);
	if (!pa13 && pa13new) pa09 =!!(addr &0x200);
	pa13 =pa13new;
}

int	MAPINT	interceptPPURead (int bank, int addr) { // Nanjing
	checkCHRSplit(bank, addr);
	if (reg[0] &0x80 && !pa13)
		return dummyPPU(bank &3 | (pa09? 4: 0), addr);
	else
		return dummyPPU(bank, addr);
}

void	applyMapper () {
	for (int bank =0x6; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, cartRead);
	for (int bank =0x6; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, cartWrite);
	for (int bank =0x0; bank <=0x7; bank++) EMU->SetPPUReadHandler(bank, dummyPPU);

	mapperCPUCycle =NULL;
	mapperPPUCycle =NULL;
	GenSound =NULL;
	uint8_t fusemap =ROM->INES2_SubMapper <<4 | reg[7] >>4 &0xF;
	uint8_t flags =reg[7] &0xF;
	switch(fusemap) {
		case 0x50:
			prgAND =flags &2? (extra &2? 0x07: 0x0F): 0x1F;
			mapperSync =syncFME7;
			mapperCPUCycle =FME7::cpuCycle;
			GenSound =SUN5sound::Get;
			FME7::reset(RESET_HARD);
			break;		
		case 0x00: case 0x01: case 0x32:	/* SxROM, SUROM */
			prgAND =flags &2? (flags &8? 0x03: 0x07): 0x0F;
			mapperSync =fusemap &0x01? syncSUROM: syncMMC1;
			mapperCPUCycle =MMC1::cpuCycle;
			MMC1::reset(RESET_HARD);
			break;
		case 0x0A:	/* PNROM */
			prgAND =0x0F;
			mapperSync =syncMMC2;
			MMC2::reset(RESET_HARD);
			break;
		default:
			EMU->DbgOut(L"Unknown fusemap %02X", fusemap);
		case 0x10: case 0x11: case 0x12:	/* TxROM, TxLROM */
			prgAND =flags &8? (flags &4? (flags &2? (extra &2? 0x07: 0x0F): 0x1F): 0x3F): 0x7F;
			mapperSync =fusemap ==0x12? syncMMC3_OneScreenMirroring: syncMMC3;
			mapperCPUCycle =MMC3::cpuCycle;
			mapperPPUCycle =MMC3::ppuCycle;
			MMC3::reset(RESET_SOFT); // Soft reset to preserve Mirroring
			MMC3::enableIRQ =false;
			break;
		case 0x08: /* FxROM */
			prgAND =flags &2? 0x07: 0x0F;
			mapperSync =syncMMC4;
			MMC4::reset(RESET_HARD);
			break;
		case 0x09: case 0x0B: case 0x17: case 0x37:  /* UxROM */
			prgAND =fusemap ==0x0B || fusemap ==0x17 &&~reg[7] &2? 0x1F: reg[7] &2? 0x07: 0x0F;
			if (flags &8) {
				mapperSync =syncUNROM;
				Latch::reset(RESET_HARD);
			} else { /* Holy Diver */
				mapperSync =syncIF12;
				for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeIF12_CHR);
				for (int bank =0xC; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIF12_PRG);
			}
			break;
		case 0x04: case 0x06: case 0x14: case 0x16: /* AxROM, BxROM */
			prgAND =(fusemap ==0x06 || fusemap ==0x16)? 0x0F: flags &2? 0x03: 0x07;
			mapperSync =flags &8? syncBNROM: syncANROM;
			Latch::reset(RESET_HARD);
			break;
		case 0x05: case 0x15: /* NROM, CNROM, BF9097 */
			for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeCNROM_CHR);
			for (int bank =0xE; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeCNROM_PRG);
			mapperSync =syncCNROM;
			if (!flags) EMU->SetCPUWriteHandler(0x9, writeCNROM_Mirroring);
			reg[0] =reg[1] =reg[2] =0;
			break;
		case 0x0C: case 0x0D: case 0x1C: case 0x1D: /* GNROM/Color Dreams */
			prgAND =flags &8? 1: 3;
			mapperSync =reg[6] &0x1000 && fusemap &~0x10? syncGNROM: syncColorDreams;
			Latch::reset(RESET_HARD);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: /* Konami VRC2/4 */
			prgAND =flags &2? 0x0F: 0x1F;
			VRC24::A0 =fusemap &2? 2: 1;
			VRC24::A1 =fusemap &2? 1: 2;
			VRC24::vrc4 =!!(fusemap &1);
			mapperCPUCycle =VRC24::cpuCycle;
			mapperSync =syncVRC24;
			VRC24::reset(RESET_HARD);
			break;
		case 0x30: case 0x31: /* Konami VRC6 */
			prgAND =flags &2? 0x0F: 0x1F;
			mapperCPUCycle =VRC6::cpuCycle;
			mapperSync =syncVRC6;
			GenSound =VRC6::mapperSnd;
			VRC6::reset(RESET_HARD);
			break;
		case 0x40: /* Konami VRC1 */
			prgAND =flags &8? (flags &4? (flags &2? 0x0F: 0x1F): 0x3F): 0x7F;
			mapperSync =syncVRC1;
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeVRC1);
			break;
		case 0x41: /* Konami VRC7 */
			prgAND =flags &8? (flags &4? (flags &2? 0x0F: 0x1F): 0x3F): 0x7F;
			mapperCPUCycle =VRC7::cpuCycle;
			mapperSync =syncVRC7;
			GenSound =VRC7::mapperSnd;
			VRC7::reset(RESET_HARD);
			break;
		case 0x44: /* Konami VRC3 */
			prgAND =0x07;
			mapperSync =syncVRC3;
			mapperCPUCycle =cpuCycle_VRC3;
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeVRC3);
			break;
		case 0x0E: case 0x1E: /* Nanjing */
			reg[0] =reg[1] =reg[2] =reg[3] =0;
			mapperSync =syncNanjing;
			for (int bank =0; bank <12; bank++) {
				EMU->SetPPUReadHandler(bank, interceptPPURead);
				EMU->SetPPUReadHandlerDebug(bank, dummyPPU);
			}
			break;			
		case 0x07: /* SMB2J */
			for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeSMB2J_IRQ);
			for (int bank =0xE; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeSMB2J_PRG);
			reg[0] =reg[1] =reg[2] =0;
			mapperCPUCycle =cpuCycle_SMB2J;
			mapperSync =syncSMB2J;
			break;
	}
}

int	MAPINT	readReg (int bank, int addr) {
	switch(addr) {
		case 0x030:
			return 0x02;
		case 0x301: case 0x601:
			return scratchROM.getData()*0x80 | *EMU->OpenBus &~0x80;
		default:
			return *EMU->OpenBus;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (reg[6] &0x8000 && addr >=0x600) return;
	if (addr >=0x700 && addr &2) {
		extra =val;
		applyMapper();
		sync();
		return;
	}
	int index =addr >>8 &7;
	if (addr &1)
		reg[index] =reg[index] &0x00FF | val <<8;
	else
		reg[index] =reg[index] &0xFF00 | val &0xFF;
	if (index ==7 && ~addr &1) applyMapper();
	if (index ==3 && ROM->INES2_SubMapper ==0) scratchROM.setPins(!!(reg[3] &0x0400), !!(reg[3] &0x0200), !!(reg[3] &0x0100));	
	if (index ==6 && ROM->INES2_SubMapper ==1) scratchROM.setPins(!!(reg[6] &0x1000), !!(reg[6] &0x0200), !!(reg[6] &0x0100));
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	
	MMC1::load(sync, MMC1Type::MMC1B);
	MMC2::load(sync);
	MMC3::load(sync);
	MMC4::load(sync);
	VRC24::load(sync, true, 1, 2, NULL, true, 0);
	VRC6::load(sync, 1, 2);
	VRC7::load(sync, 0x10, 0x20);
	Latch::load(sync, false);
	FME7::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg[6] =0x0000;
	reg[7] =0xFF0F;
	extra =0x10;

	cartRead =EMU->GetCPUReadHandler(0x8);
	cartWrite =EMU->GetCPUWriteHandler(0x8);
	dummyPPU =EMU->GetPPUReadHandler(0x0);
	EMU->SetCPUReadHandler(0x5, readReg);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	
	applyMapper();
	sync();
}

void	MAPINT	unload (void) {
	VRC7::unload();
}

void	MAPINT	cpuCycle (void) {
	if (mapperCPUCycle) mapperCPUCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (mapperPPUCycle) mapperPPUCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg)         SAVELOAD_WORD(stateMode, offset, data, c);
	for (auto& c: scratchData) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) applyMapper();
	offset =MMC1::saveLoad(stateMode, offset, data);
	offset =MMC2::saveLoad(stateMode, offset, data);
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =MMC4::saveLoad(stateMode, offset, data);
	offset =VRC24::saveLoad(stateMode, offset, data);
	offset =VRC6::saveLoad(stateMode, offset, data);
	offset =VRC7::saveLoad(stateMode, offset, data);
	offset =Latch::saveLoad_AD(stateMode, offset, data);	
	for (auto& c: vrc1Reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int cycles) {
	if (GenSound && *EMU->BootlegExpansionAudio)
		return GenSound(cycles);
	else
		return 0;
}

uint16_t mapperNum =468;
} // namespace

MapperInfo MapperInfo_468 ={
	&mapperNum,
	_T("BlazePro FPGA"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	ppuCycle,
	saveLoad,
	mapperSnd,
	NULL
};