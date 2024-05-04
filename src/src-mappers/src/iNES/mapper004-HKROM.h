#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC6.h"

namespace HKROM {
void	sync (void) {
	MMC6::syncWRAM();
	MMC6::syncPRG(0x3F, 0);
	MMC6::syncCHR(0xFF, 0);
	MMC6::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC6::load(sync);
	return TRUE;
}

uint16_t mapperNum =4;
MapperInfo MapperInfo_HKROM ={
	&mapperNum,
	_T("Nintendo HKROM"),
	COMPAT_FULL,
	load,
	MMC6::reset,
	::unload,
	MMC6::cpuCycle,
	MMC6::ppuCycle,
	MMC6::saveLoad,
	NULL,
	NULL
};
} // namespace
