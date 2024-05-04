#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	unload (void);
}
#include	"mapper111-GTROM.h"
#include	"MMC1-based\mapper111-cMMC1.h"

namespace {
BOOL	MAPINT	load (void) {
	if (ROM->CHRROMSize)
		memcpy(&MapperInfo_111, &cMMC1::MapperInfo_111, sizeof(MapperInfo));
	else
		memcpy(&MapperInfo_111, &GTROM::MapperInfo_111, sizeof(MapperInfo));
	if (MapperInfo_111.Load)
		return MapperInfo_111.Load();
	else
		return TRUE;
}

void	MAPINT	unload (void) {
	MapperInfo_042.Load =load;
}

uint16_t mapperNum =111;
} // namespace

MapperInfo MapperInfo_111 ={
	&mapperNum,
	_T("GTROM/Chinese MMC1"),
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