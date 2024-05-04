#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_EEPROM_93Cx6.h"

#define mode1bpp !!(reg[0] &0x80)
#define prgLow     (reg[0] &0x0F | reg[0] >>1 &0x10)
#define prgHigh    (reg[1] <<5)
#define mode       (reg[0] >>5 &2 | reg[0] >>4 &1)
#define mirrorH  !!(reg[0] &0x10 && ~reg[3] &0x80)

namespace {
uint8_t		reg[8];
bool		pa00;
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
	if (EEPROM) EEPROM->write(!!(reg[2] &0x10), !!(reg[2] &0x04), !!(reg[2] &0x01));

	switch (mode) {
		case 0: // UNROM-512
			EMU->SetPRG_ROM16(0x8, prgHigh | prgLow);
			EMU->SetPRG_ROM16(0xC, prgHigh |   0x1F);
			break;
		case 1: // Open Bus
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
			break;
		case 2:	// UNROM-448+64
			EMU->SetPRG_ROM16(0x8, prgHigh | prgLow);
			EMU->SetPRG_ROM16(0xC, prgHigh |(prgLow >=0x1C? 0x1C: 0x1E));
			break;
		case 3:	if (prgLow &0x10) { // 2xUNROM-128
				EMU->SetPRG_ROM16(0x8, prgHigh | prgLow &0x0F | prgLow <<1 &0x10);
				EMU->SetPRG_ROM16(0xC, prgHigh |         0x0F | prgLow <<1 &0x10);
			} else // BNROM-512
				EMU->SetPRG_ROM32(0x8, prgHigh >>1 | prgLow);
	}
	
	EMU->SetCHR_RAM8(0x0, 0);
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readASIC (int bank, int addr) {
	if (addr &0x800 || ~addr &0x400)
		return *EMU->OpenBus;
	else
	if (EEPROM)
		return EEPROM->read()? 0x00: 0x04;
	else
		return reg[2] &0x04;
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (addr &0x800)
		;
	else {
		int index =addr >>8 &7;
		reg[index] =addr &0x400? *EMU->OpenBus: val;
		sync();
	}
}

void	checkMode1bpp (int bank, int addr) {
	// During rising edge of PPU A13, PPU A9 is latched.
	bool pa13new =!!(bank &8);
	if (!pa13 && pa13new) {
		pa00 =!!(addr &0x001);
		pa09 =!!(addr &0x200);
	}
	pa13 =pa13new;
}

int	MAPINT	interceptPPURead (int bank, int addr) {
	checkMode1bpp(bank, addr);
	if (mode1bpp && !pa13)
		return readPPU(bank &3 | (pa09? 4: 0), addr &~8 | (pa00? 8: 0));
	else
		return readPPU(bank, addr);
}

void	MAPINT	interceptPPUWrite (int bank, int addr, int val) {
	checkMode1bpp(bank, addr);
	writePPU(bank, addr, val);
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
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	pa00 =false;
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
	SAVELOAD_BOOL(stateMode, offset, data, pa00);
	SAVELOAD_BOOL(stateMode, offset, data, pa09);
	SAVELOAD_BOOL(stateMode, offset, data, pa13);
	if (EEPROM) offset =EEPROM->saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =164;
} // namespace

MapperInfo MapperInfo_164 = {
	&mapperNum,
	_T("燕城 cy2000-3"),
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