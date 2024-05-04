#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	unload (void);
}

#include	"MMC3-based\mapper004-TxROM.h"
#include	"mapper004-HKROM.h"
#include	"MMC3-based\mapper004-T9552.h"

namespace {
BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper ==5)
		memcpy(&MapperInfo_004, &T9552::MapperInfo_T9552, sizeof(MapperInfo));
	else
	if (ROM->INES2_SubMapper ==1)
		memcpy(&MapperInfo_004, &HKROM::MapperInfo_HKROM, sizeof(MapperInfo));
	else
		memcpy(&MapperInfo_004, &TxROM::MapperInfo_TxROM, sizeof(MapperInfo));
	return MapperInfo_004.Load();
}

void	MAPINT	unload (void) {
	MapperInfo_004.Load =load;
}

uint16_t mapperNum =4;
uint16_t mapperNum2 =249;
} // namespace

MapperInfo MapperInfo_004 ={
	&mapperNum,
	_T("Nintendo TxROM/HKROM"),
	COMPAT_FULL,
	load,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

MapperInfo MapperInfo_249 ={
	&mapperNum,
	_T("43-319"),
	COMPAT_FULL,
	T9552::load,
	T9552::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	T9552::saveLoad,
	NULL,
	NULL
};
