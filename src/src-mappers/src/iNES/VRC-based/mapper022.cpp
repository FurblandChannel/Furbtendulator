#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, VRC24::chr[i] >>1);
	VRC24::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, false, 0x02, 0x01, NULL, true, 0);
	return TRUE;
}

uint16_t mapperNum =22;
} // namespace

MapperInfo MapperInfo_022 = {
	&mapperNum,
	_T("Konami 351618"),
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