#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM4(0, Latch::addr &1 | Latch::addr <<1 &2);
	EMU->SetCHR_ROM4(4, Latch::addr &1 | Latch::addr <<1 &2);
	if (Latch::addr &1)
		EMU->Mirror_S1();
	else
		EMU->Mirror_S0();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =27;
} // namespace

MapperInfo MapperInfo_027 ={
	&mapperNum,
	_T("CC-21"),
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