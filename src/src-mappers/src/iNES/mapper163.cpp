#include	"..\DLL\d_iNES.h"

#define useA15A16 !!(reg[3] &0x04)
#define swapBits  !!(reg[3] &0x01)
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
	EMU->SetPRG_ROM32(0x8,
	         reg[2] <<4 | reg[0] &0x0F                       // PRG A22..A15
	      | (reg[3] &4? 0x00: 0x03)                          // PRG A16..A15 fixed
	);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

int	MAPINT	readASIC (int bank, int addr) {
	if (~addr &0x800)
		return ~reg[1] &0x04;
	else
		return *EMU->OpenBus;
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (~addr &0x800) {
		int index =addr >>8 &3;

		// Swap the bits of registers $5000-$5200 if that is enabled.
		// However, exclude $5200 from the bit swap if there are only 1 MiB of PRG-ROM because PRG A20 is always 0.
		if (swapBits && index <=(ROM->PRGROMSize <2*1024*1024? 1: 2)) val =val &~3 | val <<1 &2 | val >>1 &1;

		if (addr &1) {
			if (reg[1] &0x01 && ~val &0x01) reg[1] ^=0x04;  // If A0=1, flip feedback bit on falling edges of D0
		} else {						// If A0=0, write to register
			reg[index] =val;
			sync();
		}
	}
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

uint16_t mapperNum =163;
} // namespace

MapperInfo MapperInfo_163 = {
	&mapperNum,
	_T("南晶 FC-001"),
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
