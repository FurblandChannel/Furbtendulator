#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_MMC1.h"
#include	"..\Hardware\h_VS.h"

namespace {
void	sync (void) {
	MMC1::syncWRAM(0);
	MMC1::syncPRG(0x0F, 0);
	MMC1::syncCHR(0x1F, 0);
	MMC1::syncMirror();
}

BOOL	MAPINT	load (void) {
	VS::Load();
	MMC1::load(sync, MMC1Type::MMC1B);
	iNES_SetSRAM();
	return TRUE;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	VS::Reset(resetType);
	MMC1::reset(resetType);
}
void	MAPINT	unload (void) {
	VS::Unload();
}

void	MAPINT	cpuCycle (void) {
	MMC1::cpuCycle();
	VS::CPUCycle();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VS::SaveLoad(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	return offset;
}

uint16_t MapperNum = 1;
} // namespace

MapperInfo MapperInfo_001 =
{
	&MapperNum,
	_T("MMC1"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};