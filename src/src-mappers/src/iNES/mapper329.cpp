#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, Latch::data >>6);
	EMU->SetPRG_ROM32(0x8, Latch::data &0x1F);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();	
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(ResetType);
}

uint16_t MapperNum =329;
} // namespace

MapperInfo MapperInfo_329 ={
	&MapperNum,
	_T("EDU2000"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
