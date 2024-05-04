#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>6 &0x0E | Latch::addr >>5 &1)
#define chr       (Latch::addr &0x0F)
#define nrom256 !!(Latch::addr &0x40)
#define mirrorH !!(Latch::addr &0x10)

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

uint16_t mapperNum =261;
} // namespace

MapperInfo MapperInfo_261 = {
	&mapperNum,
	_T("810544-C-A1/NTDEC 2746"),
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