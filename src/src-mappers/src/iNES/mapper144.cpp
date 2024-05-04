#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	uint8_t Value =(EMU->GetCPUReadHandler(Latch::addr >>12))(Latch::addr >>12, Latch::addr &0xFFF) & (Latch::data |1);
	EMU->SetPRG_ROM32(0x8, (Value >> 0) & 0x3);
	iNES_SetCHR_Auto8(0x0, (Value >> 4) & 0xF);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, TRUE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(ResetType);
}

uint16_t MapperNum = 144;
} // namespace

MapperInfo MapperInfo_144 = {
	&MapperNum,
	_T("AGCI-50282"),
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
