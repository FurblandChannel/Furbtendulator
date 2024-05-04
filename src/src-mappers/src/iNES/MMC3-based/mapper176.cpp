#include "..\..\DLL\d_iNES.h"

/*	Mappers:
	176 - Standard
	523 - Jncota KT-xxx (1 KiB->2 KiB, 2 KiB->4 KiB CHR, hard-wired nametable mirroring)	

	Submappers:	
	0 - Standard
	1 - FK-xxx
	2 - 外星 FS005/FS006
	3 - JX9003B
	4 - GameStar Smart Genius Deluxe
	5 - HST-162 (500-in-1)
	6 - FZZC3Y
	
	Verified on real hardware:
	"Legend of Kage" sets CNROM latch 1 and switches between CHR bank 0 and 1 using 5FF2, causing the wrong bank (1 instead of 0) during gameplay.
	
	Heuristics for NES 1.0:
	- 1 MiB PRG+1 MiB CHR     => Submapper 1
	- 256 KiB PRG+128 KiB CHR => Submapper 1
	- 128 KiB PRG+64 KiB CHR  => Submapper 1
	- A001.5 ever set         => Submapper 2
	- 5FF5/5FF6 written-to    => Submapper 3
*/

namespace {
FCPUWrite	writeAPU;
FCPUWrite	writeWRAM;

uint8_t		pointer;
uint8_t		mmc3reg[16];
uint8_t		fk23reg[8];
uint8_t		mirroring;
uint8_t		wram;
uint8_t		latch;
uint8_t		reg4800;

uint8_t		counter;
uint8_t		reloadValue;
uint8_t		pa12Filter;
bool		enableIRQ;

#define		prgInvert !!(pointer &0x40)
#define		chrInvert !!(pointer &0x80)

#define		prgMode     (fk23reg[0] &7)

#define		chrMixed  ROM->INES2_SubMapper ==2 && !!(wram &0x04)
#define		chrNROM  (!!(fk23reg[0] &0x60) || ROM->CHRROMSize ==0 && ROM->CHRRAMSize ==8192)
#define		chrCNROM ( !(fk23reg[0] &0x20) && (ROM->INES2_SubMapper ==1 || ROM->INES2_SubMapper ==5))
#define		chrSmall  !!(fk23reg[0] &0x10)
#define		chrRAM   (!!(fk23reg[0] &0x20) && !(fk23reg[0] &0x40) && ROM->CHRRAMSize >0 || ROM->CHRROMSize ==0)

#define		wramEnabled   !!(wram &0x80)
#define		enableRegs   (ROM->INES2_SubMapper !=2 || !!(wram &0x40))
#define		mmc3Extended (!!(fk23reg[3] &0x02) && (ROM->INES2_SubMapper ==1 || ROM->INES2_SubMapper ==2 || ROM->INES_MapperNum ==523))

int	getPRGBank (int bank) {
	const static int prgModeAND[8] ={ 0x3F, 0x1F, 0x0F, 0x00, 0x00, 0x00, 0x7F, 0xFF};
	int prgAND =prgModeAND[prgMode];
	int prgOR  =fk23reg[1] &0x07F;
	switch (ROM->INES2_SubMapper) {
		case 1: // FK-xxx
			if (prgMode ==0 && mmc3Extended) prgAND =0xFF; // 2 MiB MMC3
			break;
		case 2: // Waixing FS005
			prgOR |=fk23reg[0] <<4 &0x080 |
			        fk23reg[0] <<1 &0x100 |
			        fk23reg[2] <<3 &0x600 |
			        fk23reg[2] <<6 &0x800;
			break;
		case 3: // Super Mario 160 Funny Time: Enhanced variant with extra MSB register
			prgOR |=fk23reg[5] <<7;
			if (prgMode ==0) prgAND =0xFF; // 2 MiB MMC3
			break;
		case 4: // GameStar Smart Genius Deluxe
			prgOR |=fk23reg[2] &0x080;
			break;
		case 5:	// HST-162
			prgOR =prgOR &0x1F | reg4800 <<5;
			break;
	}
		
	*EMU->multiCanSave  =TRUE;
	*EMU->multiPRGStart =ROM->PRGROMData +prgOR*16384;
	*EMU->multiPRGSize  =prgMode <3? (prgAND +1)*8192: prgMode ==3? 16384: 32768;
	
	switch(prgMode) {
		case 3:	// NROM-128
			return prgOR <<1     | bank &1;
		case 4:	// NROM-256
			return prgOR <<1 &~3 | bank;
		case 5: // UNROM
			return prgOR <<1 &~15| latch <<1 &14 | bank &1 | (bank &2? 14: 0);
		case 6: // Invalid, verified to force bank 0
		case 7:	return 0;
		default: // MMC3 with 512/256/128 KiB PRG
			prgOR <<=1;
			if (~bank &1 && prgInvert) bank ^=2;
			return (!mmc3Extended && bank &2? (0xFE | bank &1): mmc3reg[6 +bank]) &prgAND | prgOR &~prgAND;
	}
}

int	getCHRBank (int bank) {
	int chrOR  =fk23reg[2] <<3;
	int chrAND =chrSmall? 0x7F: 0xFF;
	if (ROM->INES2_SubMapper ==3) chrOR |=fk23reg[6] <<11; // Super Mario 160 Funny Time: Enhanced variant with extra MSB register
	
	if (chrRAM)
		*EMU->multiCHRStart =ROM->CHRRAMData +((chrOR *1024) &(ROM->CHRRAMSize -1));
	else
		*EMU->multiCHRStart =ROM->CHRROMData +((chrOR *1024) &(ROM->CHRROMSize -1));
	
	*EMU->multiCHRSize  =(chrNROM? (chrCNROM? (chrSmall? 16: 32): 8): (chrSmall? 128: 256)) *1024;
	
	if (chrNROM) {
		int mask =chrCNROM? (chrSmall? 0x08: 0x18): 0x00;
		return (ROM->INES2_SubMapper ==5? chrOR: (chrOR &~mask)) | (latch <<3) &mask | bank;
	} else {
		const static int bank2ext[8] ={  0, 10,  1, 11,  2,  3,  4,  5 };
		const static int bank2reg[8] ={  0,  0,  1,  1,  2,  3,  4,  5 };
		const static int bankAND [8] ={ ~1, ~1, ~1, ~1, ~0, ~0, ~0, ~0 };
		const static int bankOR  [8] ={  0,  1,  0,  1,  0,  0,  0,  0 };
		if (chrInvert) bank ^=4;
		if (mmc3Extended)
			return mmc3reg[bank2ext[bank]] &chrAND                              |chrOR &~chrAND;
		else
			return mmc3reg[bank2reg[bank]] &chrAND &bankAND[bank] |bankOR[bank] |chrOR &~chrAND;
	}
}

/* Waixing FS005/FS006 has extended WRAM behavior */
void	sync() {
	if (wramEnabled) {
		if (ROM->INES2_SubMapper ==2) {
			switch(wram >>5 &3) { /* Behavior of A001.5-6 verified by Krzysiobal */
			case 0:	EMU->SetPRG_RAM16(0x4, 0);
				break;
			case 1:	EMU->SetPRG_RAM16(0x4, wram &1);
				break;
			case 2:	EMU->SetPRG_OB4(0x5);
				EMU->SetPRG_RAM8(0x6, 0);
				break;
			case 3:	EMU->SetPRG_OB4 (0x5);
				EMU->SetPRG_RAM8(0x6, wram &3);
			}
		} else {
			EMU->SetPRG_OB4(0x5);
			EMU->SetPRG_RAM8(0x6, 0);
		}
	} else {
		EMU->SetPRG_OB4(0x5);
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
	EMU->SetPRG_OB4(0x4); /* SetPRG_RAM16 may have set the $4000-$4FFF range to RAM, a notation used for convenience, so undo this if necessary. */

	for (int bank =0; bank <4; bank ++) EMU->SetPRG_ROM8(0x8 +bank*2, getPRGBank(bank));
	
	if (ROM->INES_MapperNum ==523) 
		for (int bank =0; bank <8; bank +=2) EMU->SetCHR_ROM2(bank, getCHRBank(bank));
	else
		for (int bank =0; bank <8; bank++) {
			int val =getCHRBank(bank);
			if (chrRAM || chrMixed && val <0x08)
				EMU->SetCHR_RAM1(bank, val);
			else
				EMU->SetCHR_ROM1(bank, val);
		}

	switch (mirroring &(ROM->INES2_SubMapper ==2? 3: 1)) {
		case 0:	EMU->Mirror_V(); break;
		case 1: EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeWRAM(bank, addr, val); // Will go to nowhere if no WRAM has been mapped via EMU->SetPRG_xxx
	if (enableRegs && addr &ROM->dipValue) {
		int index =addr &(ROM->INES2_SubMapper ==3? 7: 3);
		fk23reg[index] =val;
		sync();
	}
}

void	MAPINT	writeMMC3 (int bank, int addr, int val) {
	switch (addr &3) {
		case 0:
			pointer =val;		
			if (ROM->INES2_SubMapper ==2 && prgInvert && (val &0x07) >=6) pointer ^=1; // FS005 swaps registers $46 and $47
			break;
		case 1:
			mmc3reg[pointer &(mmc3Extended? 0xF: 0x7)] =val;
			break;
		case 2: // Confirmed on real hardware: MMC3 address mask is $E003 instead of the correct $E001
		case 3:
			break;
	}
	latch =val;
	sync();
}

void	MAPINT	writeMirroringWRAM (int bank, int addr, int val) {
	if (addr &1)
		wram =val;
	else
		mirroring =val;
	
	latch =val;
	sync();
}

void	MAPINT	writeIRQConfig (int bank, int addr, int val) {
	if (addr &1)
		counter =0;
	else
		reloadValue =val;

	latch =val;
	sync();
}

void	MAPINT	writeIRQEnable (int bank, int addr, int val) {
	enableIRQ =!!(addr &1);
	if (!enableIRQ) EMU->SetIRQ(1);

	latch =val;
	sync();
}

void	MAPINT	write4800 (int bank, int addr, int val) {
	if (addr &0x800) {
		reg4800 =val;
		sync();
	} else
		writeAPU(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();		
	if (ROM->INES_Version <2) {
		if (CART_BATTERY) { ROM->INES2_SubMapper =2; EMU->SetPRGRAMSize(32768); } else // Only Waixing games have batteries
		if (ROM->PRGROMSize ==1024*1024 && ROM->CHRROMSize ==1024*1024) ROM->INES2_SubMapper =1; else
		if (ROM->PRGROMSize == 256*1024 && ROM->CHRROMSize == 128*1024) ROM->INES2_SubMapper =1; else
		if (ROM->PRGROMSize >=8192*1024 && ROM->CHRROMSize ==   0*1024) ROM->INES2_SubMapper =2; else
		if (ROM->PRGROMSize ==4096*1024 && ROM->CHRROMSize ==   0*1024) ROM->INES2_SubMapper =3;
	}
	
	MapperInfo_176.Description =_T("YH-xxx");
	switch(ROM->INES2_SubMapper) {
		case 0:
			if (ROM->CHRROMSize && ROM->CHRRAMSize ==8192) MapperInfo_176.Description =_T("SFC-12B");
			break;
		case 1:
			MapperInfo_176.Description =_T("FK-xxx");
			break;
		case 2:
			MapperInfo_176.Description =_T("外星 FS005");
			break;
		case 3:
			MapperInfo_176.Description =_T("JX9003B");
			break;
		case 4:
			MapperInfo_176.Description =_T("GameStar Smart Genius Deluxe");
			break;
		case 5:
			MapperInfo_176.Description =_T("HST-162");
			break;
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	// Cart detects soft reset, so always do hard reset
	if (resetType ==RESET_HARD) {
		if (!ROM->dipValueSet) ROM->dipValue =0x010;
		if (ROM->INES_Version <2) { 
			// Auto-correct CHR-RAM size, which is important for the proper interpretation of some register bits
			if (ROM->CHRROMSize) {
				if (ROM->PRGROMSize ==2048*1024 && ROM->CHRROMSize ==512*1024)	// Rockman I-VI
					EMU->SetCHRRAMSize(8*1024);
				else
					EMU->SetCHRRAMSize(0);
			} else switch (ROM->PRGROMSize) {	// No CHR-ROM. Detect CHR-RAM size by looking at PRG-ROM size. PRG-ROMs larger than 4 MiB are not possible without NES 2.0 anyway.
				case 2048*1024:	// 10-in-1 Omake Game
				case 4096*1024: // Super Mario 160-in-1 Funny Time
					EMU->SetCHRRAMSize(128*1024);
					break;
				default:	// Normal oversize TNROM
					EMU->SetCHRRAMSize(8*1024);	
					break;
			}		
		}
	}

	static const uint8_t initialReg[12] = { 0x00, 0x02, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0xFE, 0xFF, 0x01, 0x03 };
	for (int i =0; i <12; i++) mmc3reg[i] = initialReg[i];
	for (auto& r: fk23reg) r =0x00;
	if (ROM->INES2_SubMapper ==1) fk23reg[1] =0xFF;
	
	if (ROM->INES_MapperNum ==523) 
		mirroring =(ROM->INES_Flags &1) ^1;
	else
		mirroring =0;
	wram =ROM->INES2_SubMapper ==2? 0xC0: 0x80;
	
	pointer =0;
	counter =0;
	reloadValue =0;
	pa12Filter =0;
	latch =0;
	reg4800 =0;
	enableIRQ =false;

	EMU->SetIRQ(1);
	sync();

	writeWRAM =EMU->GetCPUWriteHandler(0x6);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeMMC3);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeMirroringWRAM);
	for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeIRQConfig);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIRQEnable);
	if (ROM->INES2_SubMapper ==5) {
		writeAPU =EMU->GetCPUWriteHandler(0x4);
		EMU->SetCPUWriteHandler(0x4, write4800);
	}
}

void	MAPINT	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			counter =!counter? reloadValue: --counter;
			if (!counter && enableIRQ) EMU->SetIRQ(0);
		}
		pa12Filter =5;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0; i <12; i++) SAVELOAD_BYTE(stateMode, offset, data, mmc3reg[i]); // registers 12-15 are unused
	for (auto& r: fk23reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, wram);
	SAVELOAD_BYTE(stateMode, offset, data, latch);

	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);

	if (ROM->INES2_SubMapper ==5) SAVELOAD_BYTE(stateMode, offset, data, reg4800);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum176 =176;
uint16_t mapperNum523 =523;
} // namespace

MapperInfo MapperInfo_176 ={
	&mapperNum176,
	_T("外星 FS005/FS006/FK23C(A)"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_523 ={
	&mapperNum176,
	_T("晶科泰 封神榜"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
