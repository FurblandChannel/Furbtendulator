#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	N118::syncPRG(0x1F, 0x00); // Original hardware only supports 128 KiB, but one fan translation uses 256 KiB
	EMU->SetCHR_ROM2(0, N118::reg[2]);
	EMU->SetCHR_ROM2(2, N118::reg[3]);
	EMU->SetCHR_ROM2(4, N118::reg[4]);
	EMU->SetCHR_ROM2(6, N118::reg[5]);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =76;
} // namespace

MapperInfo MapperInfo_076 ={
	&mapperNum,
	_T("Namco 3446"),
	COMPAT_FULL,
	load,
	N118::reset,
	NULL,
	NULL,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};