#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
	EMU->SetPRG_ROM16(0xC, Latch::addr >>2);
	EMU->SetCHR_ROM8(0, Latch::addr >>2);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
}

uint16_t MapperNum =300;
} // namespace

MapperInfo MapperInfo_300 ={
	&MapperNum,
	_T("190-in-1"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};