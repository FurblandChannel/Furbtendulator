#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	N118::syncPRG(0x0F, 0x00);

	EMU->SetCHR_RAM8(0x0, 0);
	
	if (N118::reg[5] &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =418;
} // namespace

MapperInfo MapperInfo_418 ={
	&mapperNum,
	_T("820106-C/821007C"),
	COMPAT_FULL,
	load,
	N118::reset,
	NULL,
	NULL,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};
