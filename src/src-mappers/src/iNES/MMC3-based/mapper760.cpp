#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	if (ROM->MiscROMSize) EMU->SetPRG_Ptr4(0x5, ROM->MiscROMData, TRUE);
	MMC3::syncWRAM();

	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR(0xFF, 0x00);
	
	EMU->Mirror_4();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	return TRUE;
}

uint16_t mapperNum =760; /* PRELIMINARY mapper number */
} // namespace
MapperInfo MapperInfo_760 ={
	&mapperNum,
	_T("Nintendo TR1ROM"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};
