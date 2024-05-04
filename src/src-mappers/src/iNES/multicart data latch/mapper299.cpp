#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg      (Latch::data >>4 &0x07)
#define chr      (Latch::data >>2 &0x1C | Latch::data &0x03)
#define mirrorH !(Latch::data &0x80)
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =299;
} // namespace

MapperInfo MapperInfo_299 = {
	&mapperNum,
	_T("TXC 6-in-1"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
