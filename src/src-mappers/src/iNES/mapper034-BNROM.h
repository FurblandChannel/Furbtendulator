#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace BNROM {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =34;

MapperInfo MapperInfo_BNROM ={
	&mapperNum,
	_T("Nintendo BNROM"),
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
} // namespace
