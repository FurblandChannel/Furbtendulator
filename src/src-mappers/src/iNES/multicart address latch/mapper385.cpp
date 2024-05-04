#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
	EMU->SetPRG_ROM16(0xC, Latch::addr >>1);
	EMU->SetCHR_RAM8(0x0, 0x0);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	Latch::reset(resetType);
}

uint16_t mapperNum =385;
} // namespace

MapperInfo MapperInfo_385 ={
	&mapperNum,
	_T("NTDEC 2779"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};