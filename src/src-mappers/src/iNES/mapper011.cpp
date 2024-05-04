#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>0);
	iNES_SetCHR_Auto8(0x0, Latch::data >>4);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false); // "Free Fall" requires FALSE, all others will run with TRUE
	return TRUE;
}

uint16_t mapperNum =11;
} // namespace

MapperInfo MapperInfo_011 ={
	&mapperNum,
	_T("Color Dreams"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
