#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		game;

void	sync (void) {
	MMC1::syncPRG(0x07, game <<3);
	MMC1::syncCHR_ROM(0x1F, game <<5);
	MMC1::syncMirror();
	MMC1::syncWRAM(0);
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1A);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		game =0;
	else
		game++;
	MMC1::reset(RESET_HARD);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =374;
} // namespace

MapperInfo MapperInfo_374 = {
	&mapperNum,
	_T("Reset-based SLROM multicart"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};

