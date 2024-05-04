#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC2.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0); // For the Playchoice 10 version
	MMC2::syncPRG(0x0F, 0x00);
	MMC2::syncCHR(0x1F, 0x00);
	MMC2::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC2::load(sync);
	return TRUE;
}

uint16_t mapperNum =9;
} // namespace

MapperInfo MapperInfo_009 ={
	&mapperNum,
	_T("Nintendo PNROM"),
	COMPAT_FULL,
	load,
	MMC2::reset,
	NULL,
	NULL,
	NULL,
	MMC2::saveLoad,
	NULL,
	NULL
};