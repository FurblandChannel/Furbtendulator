#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	sync (void) {
	VRC24::syncPRG(0x0F, VRC24::wires <<4);
	VRC24::syncCHR_ROM(0x7F, VRC24::wires <<7);
	VRC24::syncMirror();
}

BOOL	MAPINT	load (void) {
	VRC24::load(sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

uint16_t MapperNum =450;
} // namespace

MapperInfo MapperInfo_450 = {
	&MapperNum,
	_T("晶太 YY841157C"),
	COMPAT_FULL,
	load,
	VRC24::reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};