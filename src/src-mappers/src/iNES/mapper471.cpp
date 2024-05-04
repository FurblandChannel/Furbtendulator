#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
int	prevAddr;

void	sync (void) {
	EMU->SetPRG_ROM32(8, Latch::addr);
	iNES_SetCHR_Auto8(0, Latch::addr);
	iNES_SetMirroring();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	EMU->SetIRQ(1);
	Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prevAddr =0;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x8, writeLatch);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (prevAddr &0x1000 && ~addr &0x1000) EMU->SetIRQ(0);
	prevAddr =addr;
}

uint16_t mapperNum =471;
} // namespace

MapperInfo MapperInfo_471 ={
	&mapperNum,
	_T("Impact Soft IM1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	ppuCycle,
	Latch::saveLoad_AL,
	NULL,
	NULL
};