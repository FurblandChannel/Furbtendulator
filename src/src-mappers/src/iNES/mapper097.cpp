#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, 0xFF);
	EMU->SetPRG_ROM16(0xC, Latch::data);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::data &0x80)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =97;
} // namespace

MapperInfo MapperInfo_097 ={
	&mapperNum,
	_T("Irem TAM-S1"),
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