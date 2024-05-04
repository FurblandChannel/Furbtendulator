#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	unload (void);
}

#include	"mapper034-NINA001.h"
#include	"mapper034-BNROM.h"
#include	"mapper034-Nesticle34.h"

namespace {
BOOL	MAPINT	load (void) {
	if (ROM->MiscROMSize && ROM->MiscROMData)
		memcpy(&MapperInfo_034, &Nesticle34::MapperInfo_Nesticle34, sizeof(MapperInfo));
	else
	if (ROM->INES2_SubMapper ==1 || ROM->INES2_SubMapper !=2 && ROM->CHRROMSize > 0)
		memcpy(&MapperInfo_034, &NINA001::MapperInfo_NINA001, sizeof(MapperInfo));
	else
	if (ROM->INES2_SubMapper ==2 || ROM->INES2_SubMapper !=1 && ROM->CHRROMSize ==0)
		memcpy(&MapperInfo_034, &BNROM::MapperInfo_BNROM, sizeof(MapperInfo));
	
	if (MapperInfo_034.Load)
		return MapperInfo_034.Load();
	else
		return TRUE;
}

void	MAPINT	unload (void) {
	MapperInfo_034.Load =load;
}

uint16_t mapperNum =34;
} // namespace

MapperInfo MapperInfo_034 ={
	&mapperNum,
	_T("AVE NINA-001/Nintendo BNROM"),
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