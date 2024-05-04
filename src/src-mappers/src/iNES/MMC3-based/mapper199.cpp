#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {

void	sync (void) {
	EMU->SetPRG_RAM4(0x5, 2);
	MMC3::syncWRAM();
	MMC3::syncPRG(0xFF, 0);
	EMU->SetCHR_RAM8(0, 0);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

uint16_t mapperNum =199;
} // namespace

MapperInfo MapperInfo_199 = {
	&mapperNum,
	_T("外星 FS309"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};