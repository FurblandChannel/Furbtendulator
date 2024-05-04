#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>4);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	if (Latch::data &0x01)
		EMU->SetCHR_RAM8(0, 0);
	else
		for (int bank =0; bank <8; bank++) EMU->SetCHR_OB1(bank);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =93;
} // namespace

MapperInfo MapperInfo_093 ={
	&mapperNum,
	_T("Sunsoft-3R"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};