#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define nrom    !!(Latch::addr &0x080)
#define prg       (Latch::addr >>2 &0x1F)

bool		unrom;

void	sync (void) {
	if (unrom) {
		EMU->SetPRG_ROM16(0x8, 0x20 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, 0x27);
		EMU->SetCHR_RAM8(0x0, 0);
		EMU->Mirror_V();
	} else {
		EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
		EMU->SetPRG_ROM16(0xC,(prg | cpuA14) *nrom);

		EMU->SetCHR_RAM8(0x0, 0);
		if (nrom) protectCHRRAM();

		if (mirrorH)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	unrom =resetType ==RESET_HARD? false: !unrom;
	Latch::reset(RESET_HARD);
}

uint16_t mapperNum =280;
} // namespace

MapperInfo MapperInfo_280 = {
	&mapperNum,
	_T("K-3017"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};