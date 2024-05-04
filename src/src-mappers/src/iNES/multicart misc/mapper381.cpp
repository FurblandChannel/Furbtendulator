#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		game;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, ((Latch::data <<1) | (Latch::data >>4)) &0xF | (game <<4));
	EMU->SetPRG_ROM16(0xC,                                          0xF | (game <<4));
	EMU->SetCHR_RAM8(0, 0);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		game =0;
	else
		game++;
	Latch::reset(resetType);
	iNES_SetMirroring();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =381;
} // namespace

MapperInfo MapperInfo_381 = {
	&mapperNum,
	_T("KN-42"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};