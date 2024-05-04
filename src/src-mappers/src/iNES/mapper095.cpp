#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	N118::syncPRG(0x0F, 0x00);
	N118::syncCHR(0x3F, 0x00);
	EMU->SetCHR_NT1(0x8, N118::reg[0] >>5);
	EMU->SetCHR_NT1(0x9, N118::reg[0] >>5);
	EMU->SetCHR_NT1(0xA, N118::reg[1] >>5);
	EMU->SetCHR_NT1(0xB, N118::reg[1] >>5);
	EMU->SetCHR_NT1(0xC, N118::reg[2] >>5);
	EMU->SetCHR_NT1(0xD, N118::reg[3] >>5);
	EMU->SetCHR_NT1(0xE, N118::reg[4] >>5);
	EMU->SetCHR_NT1(0xF, N118::reg[5] >>5);
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =95;
} // namespace

MapperInfo MapperInfo_095 ={
	&mapperNum,
	_T("Namco 3425"),
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