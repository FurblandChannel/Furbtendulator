#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>4);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_ROM8(0, Latch::data &0xF);
	if (Latch::data &0x80)
		EMU->Mirror_S1();
	else
		EMU->Mirror_S0();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =152;
} // namespace

MapperInfo MapperInfo_152 ={
	&mapperNum,
	_T("Bandai UOROM 1SM"),
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