#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC4.h"

namespace {
void	sync (void) {
	MMC4::syncPRG(0x0F, 0x00);
	MMC4::syncCHR(0x1F, 0x00);
	MMC4::syncMirror();
	EMU->SetPRG_RAM8(0x6, 0);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->INES_CHRSize <128/8)
		MapperInfo_010.Description = _T("Nintendo FJROM"); 
	else
		MapperInfo_010.Description = _T("Nintendo FKROM"); 
	MMC4::load(sync);
	return TRUE;
}

uint16_t MapperNum =10;
} // namespace

MapperInfo MapperInfo_010 ={
	&MapperNum,
	_T("Nintendo FJROM/FKROM"),
	COMPAT_FULL,
	load,
	MMC4::reset,
	NULL,
	NULL,
	NULL,
	MMC4::saveLoad,
	NULL,
	NULL
};