#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::addr &1);
	iNES_SetCHR_Auto8(0x0, Latch::addr >>1);
	iNES_SetMirroring();
}

BOOL	MAPINT	Load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =216;
} // namespace

MapperInfo MapperInfo_216 ={
	&mapperNum,
	_T("Bonza"),
	COMPAT_PARTIAL,
	Load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};
