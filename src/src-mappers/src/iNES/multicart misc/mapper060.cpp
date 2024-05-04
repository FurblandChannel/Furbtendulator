#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		game;

void	sync (void) {
	EMU->SetCHR_ROM8(0, game);
	EMU->SetPRG_ROM16(0x8, game);
	EMU->SetPRG_ROM16(0xC, game);
	iNES_SetMirroring();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	game =resetType ==RESET_HARD? 0: ++game;
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =60;
} // namespace

MapperInfo MapperInfo_060 = {
	&mapperNum,
	_T("Reset-based NROM-128"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
