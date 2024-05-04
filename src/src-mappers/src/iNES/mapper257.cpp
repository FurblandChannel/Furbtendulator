#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	Unload (void);
}

#include	"mapper257-Small.h"
#include	"mapper257-Large.h"

namespace {
BOOL	MAPINT	Load (void) {
	if (ROM->INES2_SubMapper ==1)
		memcpy(&MapperInfo_257, &DongdaSmall::MapperInfo_DongdaSmall, sizeof(MapperInfo));
	else
	if (ROM->INES2_SubMapper ==2)
		memcpy(&MapperInfo_257, &DongdaLarge::MapperInfo_DongdaLarge, sizeof(MapperInfo));
	else
	if (ROM->PRGROMSize >=512*1024)
		memcpy(&MapperInfo_257, &DongdaLarge::MapperInfo_DongdaLarge, sizeof(MapperInfo));
	else
		memcpy(&MapperInfo_257, &DongdaSmall::MapperInfo_DongdaSmall, sizeof(MapperInfo));

	return MapperInfo_257.Load();
}

void	MAPINT	Unload (void) {
	MapperInfo_257.Load =Load;
}

uint16_t MapperNum =257;
} // namespace

MapperInfo MapperInfo_257 ={
	&MapperNum,
	_T("东达 PEC-586"),
	COMPAT_FULL,
	Load,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};