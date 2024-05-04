#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::addr >>8);
	EMU->SetCHR_ROM8(0, Latch::addr >>8);
	if (Latch::addr &(ROM->PRGROMSize &0x40000? 0x800: 0x200))
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t MapperNum =341;
} // namespace

MapperInfo MapperInfo_341 = {
	&MapperNum,
	_T("TJ-03"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};