#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>4 &0x07);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_ROM8(0, Latch::data &0x07 | Latch::data >>4 &0x08);
	if (Latch::data &0x08)
		EMU->Mirror_S1();
	else	
		EMU->Mirror_S0();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t MapperNum =89;
} // namespace

MapperInfo MapperInfo_089 ={
	&MapperNum,
	_T("Sunsoft-2"),
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