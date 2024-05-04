#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
	EMU->SetPRG_ROM16(0xC, Latch::addr >>2);
	EMU->SetCHR_ROM8(0, Latch::addr >>2);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(ResetType);
}

uint16_t MapperNum =214;
} // namespace

MapperInfo MapperInfo_214 = {
	&MapperNum,
	_T("Super Gun 20-in-1"),
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