#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>1)
#define chr       (Latch::data >>1)
#define nrom256 !!(Latch::addr &1)
#define mirrorH !!(Latch::data &1)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256);
	EMU->SetCHR_ROM8(0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =438;
} // namespace

MapperInfo MapperInfo_438 ={
	&mapperNum,
	_T("K-3071"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};