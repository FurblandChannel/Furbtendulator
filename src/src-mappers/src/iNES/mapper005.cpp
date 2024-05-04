#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC5.h"

namespace {
BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->CHRROMSize ==0)
		MapperInfo_005.Description = _T("Nintendo ENROM");
	else
	switch (ROM->PRGRAMSize) {
		case 0:		MapperInfo_005.Description =_T("Nintendo ELROM"); break;
		case 8192:	MapperInfo_005.Description =_T("Nintendo EKROM"); break;
		case 16384:	MapperInfo_005.Description =_T("Nintendo ETROM"); break;
		case 32768:	MapperInfo_005.Description =_T("Nintendo EWROM"); break;
		default:   	MapperInfo_005.Description =_T("Nintendo ExROM"); break;
	}
	MMC5::load();
	return TRUE;
}

uint16_t mapperNum =5;
} // namespace

MapperInfo MapperInfo_005 ={
	&mapperNum,
	_T("Nintendo ExROM"),
	COMPAT_FULL,
	load,
	MMC5::reset,
	NULL,
	MMC5::cpuCycle,
	NULL,
	MMC5::saveLoad,
	MMC5::mapperSound,
	NULL
};
