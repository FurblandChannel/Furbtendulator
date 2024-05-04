#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC7.h"

namespace {
void	sync (void) {
	VRC7::syncPRG(0x3F, 0x00);
	VRC7::syncCHR(0xFF, 0x00);
	VRC7::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	switch(ROM->INES2_SubMapper) {
		case 1:  VRC7::load(sync, 0x08, 0x20);
			 MapperInfo_085.Description = _T("Konami VRC7 (A3/A5)");
			 break;
		case 2:  VRC7::load(sync, 0x10, 0x20);
			 MapperInfo_085.Description = _T("Konami VRC7 (A4/A5)");
			 break;
		default: VRC7::load(sync, 0x18, 0x20);
			 break;
	}
	return TRUE;
}

uint16_t mapperNum =85;
} // namespace

MapperInfo MapperInfo_085 = {
	&mapperNum,
	_T("Konami VRC7"),
	COMPAT_FULL,
	load,
	VRC7::reset,
	VRC7::unload,
	VRC7::cpuCycle,
	NULL,
	VRC7::saveLoad,
	VRC7::mapperSnd,
	NULL
};