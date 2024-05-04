#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>2);
	EMU->SetPRG_ROM16(0xC, Latch::data >>2);
	EMU->SetCHR_ROM8(0, Latch::data &0x03);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	iNES_SetMirroring();
}

uint16_t mapperNum =203;
} // namespace

MapperInfo MapperInfo_203 ={
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