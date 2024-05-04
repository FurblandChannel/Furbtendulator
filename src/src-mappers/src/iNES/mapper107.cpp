#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	EMU->SetCHR_ROM8(0x0, Latch::addr);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =107;
} // namespace

MapperInfo MapperInfo_107 ={
	&mapperNum,
	_T("Magic Dragon"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};
