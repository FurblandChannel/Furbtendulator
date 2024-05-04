#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_MMC3.h"
#include	"..\Hardware\h_VS.h"

namespace {
void	Sync (void) {
	if (ROM->INES_Flags & 0x08)
		EMU->Mirror_4();
	else	
		MMC3::syncMirror();
	MMC3::syncWRAM();	// assume WRAM is here
	MMC3::syncPRG(0x3F, 0);
	if (ROM->INES_CHRSize)
		MMC3::syncCHR_ROM(0xFF, 0);
	else	
		MMC3::syncCHR_RAM(0x07, 0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	offset = VS::SaveLoad(mode, offset, data);
	return offset;
}

BOOL	MAPINT	Load (void) {
	VS::Load();
	MMC3::load(Sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	VS::Reset(ResetType);
	MMC3::reset(ResetType);
}

void	MAPINT	Unload (void) {
	VS::Unload();
}

void	MAPINT	cpuCycle (void) {
	MMC3::cpuCycle();
	VS::CPUCycle();
}

uint16_t MapperNum = 4;
} // namespace

MapperInfo MapperInfo_004 =
{
	&MapperNum,
	_T("MMC3"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	cpuCycle,
	MMC3::ppuCycle,
	SaveLoad,
	NULL,
	NULL
};
