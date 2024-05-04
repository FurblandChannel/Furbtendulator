#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>2);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =94;
} // namespace

MapperInfo MapperInfo_094 ={
	&mapperNum,
	_T("Nintendo UN1ROM"),
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