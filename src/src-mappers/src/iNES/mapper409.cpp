#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr);
	EMU->SetPRG_ROM16(0xC, 0xFFFF);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =409;
} // namespace

MapperInfo MapperInfo_409 ={
	&mapperNum,
	_T("retroUSB DPCMcart"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};