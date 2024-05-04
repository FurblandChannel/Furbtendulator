#include	"..\DLL\d_iNES.h"

#define a15Source !!(reg[3] &0x01)
#define a16Gate   !!(reg[3] &0x04)
#define chrSplit  !!(reg[0] &0x80)

namespace {
uint8_t		reg[4];
bool		pa09;
bool		pa13;

FPPURead	readPPU;
FPPURead	readPPUDebug;
FPPUWrite	writePPU;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);

	int prg =reg[2] <<4 | reg[0] &0x0C                            // PRG A17-A20 always normal from $5000 and $5200
			     | 0x02* !a16Gate                         // PRG A16 is 1       if $5300.2=0
			     | 0x02*  a16Gate             &reg[0]     // PRG A16 is $5000.1 if %5300.2=1
			     | 0x01*           !a15Source &reg[1] >>1 // PRG A15 is $5100.1 if               $5300.0=0
			     | 0x01* !a16Gate * a15Source             // PRG A15 is 1       if $5300.2=0 and $5300.0=1
			     | 0x01*  a16Gate * a15Source &reg[0]     // PRG A15 is $5000.0 if $5300.2=1 and $5300.0=1
	;
	EMU->SetPRG_ROM32(0x8,  prg);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

int	MAPINT	readASIC (int bank, int addr) {
	return 0x00;
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	int index =addr >>8 &3;
	reg[index] =val;
	sync();
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

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
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

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, pa09);
	SAVELOAD_BOOL(stateMode, offset, data, pa13);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =162;
} // namespace

MapperInfo MapperInfo_162 = {
	&mapperNum,
	_T("外星 FS304"),
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
