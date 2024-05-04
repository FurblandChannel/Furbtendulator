#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {

void	Sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR(0x1FF, 0x00);
	VRC24::syncMirror();
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	VRC24::load(Sync, true, 0x02, 0x01, NULL, false, 0);
	return TRUE;
}

uint16_t MapperNum =298;
} // namespace

MapperInfo MapperInfo_298 = {
	&MapperNum,
	_T("NTDEC 1201"),
	COMPAT_FULL,
	Load,
	VRC24::reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};