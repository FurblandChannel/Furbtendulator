#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>3);
	EMU->SetCHR_ROM8(0x0, Latch::data &0x7);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =148;
} // namespace

MapperInfo MapperInfo_148 ={
	&mapperNum,
	_T("聖謙 SA-008-A"),
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
