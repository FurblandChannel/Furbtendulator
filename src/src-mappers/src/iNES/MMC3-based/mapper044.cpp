#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	if ((MMC3::wram &6) ==6) {
		MMC3::syncPRG(0x1F, MMC3::wram <<4 &~0x1F);
		MMC3::syncCHR_ROM(0xFF, MMC3::wram <<7 &~0xFF);
	} else {
		MMC3::syncPRG(0x0F, MMC3::wram <<4);
		MMC3::syncCHR_ROM(0x7F, MMC3::wram <<7);
	}
	MMC3::syncWRAM();
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC3::reset(RESET_HARD);
}

uint16_t MapperNum =44;
} // namespace

MapperInfo MapperInfo_044 = {
	&MapperNum,
	_T("Super HiK 7-in-1 (MMC3)"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};