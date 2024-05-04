#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	N118::syncPRG(0x0F, 0x00);
	EMU->SetCHR_ROM2(0, N118::reg[0] >>1 &0x1F);
	EMU->SetCHR_ROM2(2, N118::reg[1] >>1 &0x1F);
	EMU->SetCHR_ROM1(4, N118::reg[2] |0x40);
	EMU->SetCHR_ROM1(5, N118::reg[3] |0x40);
	EMU->SetCHR_ROM1(6, N118::reg[4] |0x40);
	EMU->SetCHR_ROM1(7, N118::reg[5] |0x40);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =88;
} // namespace

MapperInfo MapperInfo_088 ={
	&mapperNum,
	_T("Namco 3433"),
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