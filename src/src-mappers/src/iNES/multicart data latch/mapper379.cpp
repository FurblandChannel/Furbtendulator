#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &0x03);
	EMU->SetCHR_ROM8(0x0, Latch::data >>2);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	iNES_SetMirroring();
}

uint16_t mapperNum =379;
} // namespace

MapperInfo MapperInfo_379 ={
	&mapperNum,
	_T("35-in-1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};