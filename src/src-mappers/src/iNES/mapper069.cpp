#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FME7.h"

namespace {
void	sync (void) {
	FME7::syncPRG(0x3F, 0);
	FME7::syncCHR(0xFF, 0);
	FME7::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	FME7::load(sync);
	return TRUE;
}

uint16_t MapperNum =69;
} // namespace

MapperInfo MapperInfo_069 ={
	&MapperNum,
	_T("Sunsoft-5"),
	COMPAT_FULL,
	load,
	FME7::reset,
	FME7::unload,
	FME7::cpuCycle,
	NULL,
	FME7::saveLoad,
	SUN5sound::Get,
	NULL
};