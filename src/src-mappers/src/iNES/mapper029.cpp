#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, Latch::data >>2);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	iNES_SetCHR_Auto8(0x0, Latch::data &3);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =29;
} // namespace

MapperInfo MapperInfo_029 ={
	&mapperNum,
	_T("RET-CUFROM"),
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