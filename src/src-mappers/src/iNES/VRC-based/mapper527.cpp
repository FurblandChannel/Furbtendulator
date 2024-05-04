#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	Sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFFF, 0x000);
	VRC24::syncMirror();
	EMU->Mirror_Custom(VRC24::chr[0] >> 7, VRC24::chr[0] >> 7, VRC24::chr[1] >> 7, VRC24::chr[1] >> 7);
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	VRC24::load(Sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

uint16_t MapperNum =527;
} // namespace

MapperInfo MapperInfo_527 = {
	&MapperNum,
	_T("AX-40G"),
	COMPAT_FULL,
	Load,
	VRC24::reset,
	NULL,
	NULL,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};