#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_RAM4(0, 0);
	EMU->SetCHR_RAM4(4, Latch::data);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =13;
} // namespace

MapperInfo MapperInfo_013 ={
	&mapperNum,
	_T("Nintendo CPROM"),
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