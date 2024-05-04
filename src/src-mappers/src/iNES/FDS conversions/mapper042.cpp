#include	"..\..\DLL\d_iNES.h"

namespace {
void	MAPINT	unload (void);
}
#include	"mapper042-1.h"
#include	"mapper042-2.h"
#include	"mapper042-3.h"

namespace {
BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper <1 || ROM->INES2_SubMapper >3) {
		if (ROM->INES_CHRSize)		// Ai Senshi Nicole is the only one that has CHR-ROM
			memcpy(&MapperInfo_042, &Submapper1::MapperInfo_042, sizeof(MapperInfo));
		else
		if (ROM->INES_PRGSize >128 /16) // Green Beret has 160 or 256 KiB of PRG-ROM
			memcpy(&MapperInfo_042, &Submapper2::MapperInfo_042, sizeof(MapperInfo));
		else				// Mario Baby only has 128 KiB of PRG-ROM
			memcpy(&MapperInfo_042, &Submapper3::MapperInfo_042, sizeof(MapperInfo));
	} else
	switch(ROM->INES2_SubMapper) {
		case 1: memcpy(&MapperInfo_042, &Submapper1::MapperInfo_042, sizeof(MapperInfo));
			break;
		case 2: memcpy(&MapperInfo_042, &Submapper2::MapperInfo_042, sizeof(MapperInfo));
			break;
		case 3: memcpy(&MapperInfo_042, &Submapper3::MapperInfo_042, sizeof(MapperInfo));
			break;
	}
	if (MapperInfo_042.Load)
		return MapperInfo_042.Load();
	else
		return TRUE;
}

void	MAPINT	unload (void) {
	MapperInfo_042.Load =load;
}

uint16_t mapperNum =42;
} // namespace

MapperInfo MapperInfo_042 ={
	&mapperNum,
	_T("AC08/LH09"),
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