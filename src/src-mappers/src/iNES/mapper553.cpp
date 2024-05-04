#include	"..\DLL\d_iNES.h"

namespace {
int	MAPINT	readProtection (int bank, int addr) {
	return 0x3A;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	EMU->SetPRG_ROM32(0x8, 0);
	iNES_SetCHR_Auto8(0x0, 0);
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUReadHandler(bank, readProtection);
}

uint16_t mapperNum =553;
} // namespace

MapperInfo MapperInfo_553 ={
	&mapperNum,
	_T("聖謙 3013"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};