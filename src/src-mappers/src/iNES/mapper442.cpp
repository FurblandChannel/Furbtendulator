#include	"..\DLL\d_iNES.h"

namespace {
#define mode1bpp !!(reg[0] &0x80)
#define prg        (reg[0] &0x1F | reg[0] >>1 &0x20)

uint8_t		reg[8];
bool		pa00;
bool		pa09;
bool		pa13;
FPPURead	readPPU;
FPPURead	readPPUDebug;
FPPUWrite	writePPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_RAM8(0x0, 0);
	EMU->SetPRG_RAM8(0x6, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr >>8 &7] =val;
	sync();
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
	iNES_SetSRAM();	
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	pa00 =false;
	pa09 =false;
	pa13 =false;
	sync();
	
	EMU->SetCPUWriteHandler(0x5, writeReg);	
	readPPU      =EMU->GetPPUReadHandler(0x0);
	readPPUDebug =EMU->GetPPUReadHandler(0x0);
	writePPU     =EMU->GetPPUWriteHandler(0x0);
	for (int bank =0; bank <12; bank++) {
		EMU->SetPPUReadHandler     (bank, interceptPPURead);
		EMU->SetPPUReadHandlerDebug(bank, readPPUDebug);
		EMU->SetPPUWriteHandler    (bank, interceptPPUWrite);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, pa00);
	SAVELOAD_BOOL(stateMode, offset, data, pa09);
	SAVELOAD_BOOL(stateMode, offset, data, pa13);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =442;
} // namespace

MapperInfo MapperInfo_442 ={
	&mapperNum,
	_T("Golden Key"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
