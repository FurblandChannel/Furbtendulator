#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\h_EEPROM_93Cx6.h"

#define useA15A16 !!(reg[3] &0x04)
#define swapBits  !!(reg[3] &0x02)
#define chrSplit  !!(reg[0] &0x80)
#define rom1MiB   ROM->PRGROMSize ==1*1024*1024

namespace {
uint8_t		reg[8];
bool		pa09;
bool		pa13;
EEPROM_93Cx6*	EEPROM;
uint8_t*	WRAM;

FPPURead	readPPU;
FPPURead	readPPUDebug;
FPPUWrite	writePPU;

void	sync (void) {
	if (WRAM) {
		EMU->SetPRG_Ptr4(0x6, WRAM +0x0000, TRUE);
		EMU->SetPRG_Ptr4(0x7, WRAM +0x1000, TRUE);
	} else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
	if (EEPROM) EEPROM->write(!!(reg[2] &0x04), !!(reg[2] &0x02), !!(reg[2] &0x01));

	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8,
	         reg[1] <<4 | reg[0] &0x0F                       // PRG A22..A15
	      | (reg[3] &4? 0x00: 0x03)                          // PRG A16..A15 fixed
	      | (rom1MiB && swapBits? reg[1] <<3 &0x10: 0x00)    // On 1 MiB games, connect both A19 and A20 to PRG A19
	);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

int	MAPINT	readASIC (int bank, int addr) {
	if (EEPROM) {
		return EEPROM->read()? 0x04: 0x00;
	} else
		return reg[2] &0x04;
}

void	checkCHRSplit (int bank, int addr) {
	// During rising edge of PPU A13, PPU A9 is latched.
	bool pa13new =!!(bank &8);
	if (!pa13 && pa13new) pa09 =!!(addr &0x200);
	pa13 =pa13new;
}

int	MAPINT	interceptPPURead (int bank, int addr) {
	checkCHRSplit(bank, addr);
	if (chrSplit && !pa13)
		return readPPU(bank &3 | (pa09? 4: 0), addr);
	else
		return readPPU(bank, addr);
}

void	MAPINT	interceptPPUWrite (int bank, int addr, int val) {
	checkCHRSplit(bank, addr);
	writePPU(bank, addr, val);
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (~addr &0x800) {
		// first D0/D1 swap: hard-wired
		val =val &~3 | val <<1 &2 | val >>1 &1;
		
		int index =addr >>8 &7;
		// second D0/D1 swap: by reg 3
		if (swapBits && index <3) val =val &~3 | val <<1 &2 | val >>1 &1;
		reg[index] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	size_t sizeTemp =(ROM->INES2_PRGRAM &0x0F)? (64 <<(ROM->INES2_PRGRAM &0xF)): 0;
	size_t sizeSave =(ROM->INES2_PRGRAM &0xF0)? (64 <<(ROM->INES2_PRGRAM >> 4)): 0;
	
	if (sizeSave ==512) {
		EEPROM =new EEPROM_93Cx6(ROM->PRGRAMData, 512, 8);
		WRAM =sizeTemp? ROM->PRGRAMData +sizeSave: NULL;
	} else {
		EEPROM =NULL;
		WRAM =ROM->PRGRAMSize? ROM->PRGRAMData: NULL;
	}
	iNES_SetSRAM();
	MapperInfo_558.Description =EEPROM? _T("燕城 YC-03-09"): _T("外星 FSxxx");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	pa09 =false;
	pa13 =false;
	sync();
	
	EMU->SetCPUReadHandler     (0x5, readASIC);
	EMU->SetCPUReadHandlerDebug(0x5, readASIC);
	EMU->SetCPUWriteHandler    (0x5, writeASIC);
	
	readPPU      =EMU->GetPPUReadHandler(0x0);
	readPPUDebug =EMU->GetPPUReadHandler(0x0);
	writePPU     =EMU->GetPPUWriteHandler(0x0);
	for (int bank =0; bank <12; bank++) {
		EMU->SetPPUReadHandler     (bank, interceptPPURead);
		EMU->SetPPUReadHandlerDebug(bank, readPPUDebug);
		EMU->SetPPUWriteHandler    (bank, interceptPPUWrite);
	}
}

void	MAPINT	unload (void) {
	if (EEPROM) {
		delete EEPROM;
		EEPROM =NULL;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, pa09);
	SAVELOAD_BOOL(stateMode, offset, data, pa13);
	if (EEPROM) offset =EEPROM->saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =558;
} // namespace

MapperInfo MapperInfo_558 = {
	&mapperNum,
	_T("外星 FSxxx"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
