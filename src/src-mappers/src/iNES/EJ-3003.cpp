#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

#define prg       (Latch::addr &1 | Latch::addr >>1 &6)
#define chr       (Latch::addr &1 | Latch::addr >>1 &6)
#define	cpuA14   !(Latch::addr &0x08)
#define mirrorH !!(Latch::addr &0x02)

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
	EMU->SetPRG_ROM16(0xC, prg | cpuA14);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =58;
} // namespace

MapperInfo MapperInfo ={
	&mapperNum058,
	_T("EJ-3003/820428-C"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};
