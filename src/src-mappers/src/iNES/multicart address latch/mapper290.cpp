#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>10 &0x1E | Latch::addr >>6 &0x01)
#define chr       (Latch::addr >> 5 &0x18 | Latch::addr     &0x07)
#define nrom256  !(Latch::addr &0x080)
#define mirrorH !!(Latch::addr &0x400)

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
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =290;
} // namespace

MapperInfo MapperInfo_290 = {
	&mapperNum,
	_T("Asder 20-in-1"),
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