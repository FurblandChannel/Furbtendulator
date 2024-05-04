#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC6.h"

namespace {
void	sync (void) {
	VRC6::syncPRG(0x03F, 0x000);
	VRC6::syncCHR_ROM(0x1FF, 0x000);
	VRC6::syncMirror(0x1FF, 0x000);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC6::load(sync, 0x02, 0x01);
	return TRUE;
}

uint16_t mapperNum =26;
} // namespace

MapperInfo MapperInfo_026 = {
	&mapperNum,
	_T("Konami 351949A"),
	COMPAT_FULL,
	load,
	VRC6::reset,
	VRC6::unload,
	VRC6::cpuCycle,
	NULL,
	VRC6::saveLoad,
	VRC6::mapperSnd,
	NULL
};