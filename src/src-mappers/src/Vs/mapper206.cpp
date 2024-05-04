#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_N118.h"
#include	"..\Hardware\h_VS.h"

namespace {
void	Sync (void) {
	if (ROM->INES2_SubMapper ==1)
		EMU->SetPRG_ROM32(0x8, 0x00);
	else
		N118::syncPRG(0x0F, 0x00);
	N118::syncCHR(0x3F, 0x00);
}

BOOL	MAPINT	Load (void) {
	VS::Load();
	N118::load(Sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	VS::Reset(ResetType);
	N118::reset(ResetType);
}

void	MAPINT	Unload (void) {
	VS::Unload();
}

uint16_t MapperNum =206;
} // namespace

MapperInfo MapperInfo_206 ={
	&MapperNum,
	_T("DxROM"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	VS::CPUCycle,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};