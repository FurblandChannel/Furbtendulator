#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		game;

void	sync (void) {
	switch (ROM->INES2_SubMapper) {
		default:MMC3::syncPRG(0x0F, game <<4);
			MMC3::syncCHR_ROM(0x7F, game <<7);
			break;
		case 1:	MMC3::syncPRG(0x1F, game <<5);
			MMC3::syncCHR_ROM(0x7F, game <<7);
			break;
		case 2:	MMC3::syncPRG(0x0F, game <<4);
			MMC3::syncCHR_ROM(0xFF, game <<8);
			break;
		case 3:	MMC3::syncPRG(0x1F, game <<5);
			MMC3::syncCHR_ROM(0xFF, game <<8);
			break;
		case 4:	MMC3::syncPRG(game? 0x0F: 0x1F, (game +1) <<4);
			MMC3::syncCHR_ROM(0x7F, game <<7);
			break;
	}
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		game =0;
	else
		game++;
	if (ROM->INES2_SubMapper ==4) game &=3;
	MMC3::reset(RESET_HARD);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =313;
} // namespace

MapperInfo MapperInfo_313 = {
	&mapperNum,
	_T("Reset-based TKROM multicart"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};

