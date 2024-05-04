#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, (Latch::data & 0x10) >>4);
	iNES_SetMirroring();
}

int	MAPINT	readLatch (int bank, int addr) {
	return ROM->PRGROMData[0x6000 |addr] &0xF0 | (Latch::data >>4);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	EMU->SetCPUReadHandler(0xE, readLatch);
}

uint16_t mapperNum =533;
} // namespace

MapperInfo MapperInfo_533 ={
	&mapperNum,
	_T("聖謙 3014"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
