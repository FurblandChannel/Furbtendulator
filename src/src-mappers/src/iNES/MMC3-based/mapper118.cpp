#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0x7F, 0);
	MMC3::syncMirrorA17();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->INES_Version >=2) {
		if (ROM->INES2_PRGRAM >0)
			MapperInfo_118.Description = _T("Nintendo TKSROM");
		else
			MapperInfo_118.Description = _T("Nintendo TLSROM");
	}
	MMC3::load(sync);
	return TRUE;
}

uint16_t mapperNum =118;
} // namespace

MapperInfo MapperInfo_118 ={
	&mapperNum,
	_T("Nintendo TKSROM/TLSROM"),
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