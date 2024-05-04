#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	sync (void) {
	VRC24::wires =VRC24::wires &~8 | VRC24::wires <<3 &8; // DO simply connected to DI
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR(0x1FF, 0x000);
	VRC24::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	switch(ROM->INES2_SubMapper) {
		case 1:  VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
			 MapperInfo_023.Description = _T("Konami VRC4 (A0/A1)");
			 break;
		case 2:  VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
			 MapperInfo_023.Description = _T("Konami 352396");
			 break;
		case 3:  VRC24::load(sync, false, 0x01, 0x02, NULL, true, 0);
			 MapperInfo_023.Description = _T("Konami 350603/350636/350926/351179");
			 break;
		default: VRC24::load(sync, true, 0x05, 0x0A, NULL, true, 0);
			 MapperInfo_023.Description = _T("Konami VRC2b/VRC4e/VRC4f");
			 break;
	}
	return TRUE;
}

uint16_t mapperNum =23;
} // namespace

MapperInfo MapperInfo_023 = {
	&mapperNum,
	_T("Konami VRC2b/VRC4e/VRC4f"),
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