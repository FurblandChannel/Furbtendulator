#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR(0xFFF, 0x000);
	VRC24::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	switch(ROM->INES2_SubMapper) {
		case 1:  VRC24::load(sync, true, 0x02, 0x01, NULL, true, 0);
			 MapperInfo_025.Description = _T("Konami 351406");
			 break;
		case 2:  VRC24::load(sync, true, 0x08, 0x04, NULL, true, 0);
			 MapperInfo_025.Description = _T("Konami 352400");
			 break;
		case 3:  VRC24::load(sync, false, 0x02, 0x01, NULL, true, 0);
			 MapperInfo_025.Description = _T("Konami 351948");
			 break;
		default: VRC24::load(sync, true, 0x0A, 0x05, NULL, true, 0);
			 MapperInfo_025.Description = _T("Konami VRC2c/VRC4b/VRC4d");
			 break;
	}
	return TRUE;
}

uint16_t mapperNum =25;
} // namespace

MapperInfo MapperInfo_025 = {
	&mapperNum,
	_T("Konami VRC2c/VRC4b/VRC4d"),
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