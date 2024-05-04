#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_ROM8(0x0, Latch::addr &3);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =81;
} // namespace

MapperInfo MapperInfo_081 = {
	&mapperNum,
	_T("NTDEC N715021"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};