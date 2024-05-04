#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >> 4);
	iNES_SetCHR_Auto8(0x0, Latch::data >> 0);
	if (Latch::data &0x20)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
}

uint16_t MapperNum = 271;
} // namespace

MapperInfo MapperInfo_271 =
{
	&MapperNum,
	_T("MGC-026"),
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
