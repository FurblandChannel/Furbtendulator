#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	Game;

void	Sync (void) {
	EMU->SetPRG_ROM32(0x8, Game);
	EMU->SetCHR_ROM8(0x0, Game);
	if (Game &1)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	if (ResetType ==RESET_HARD)
		Game =0;
	else
		Game++;
	(EMU->GetCPUWriteHandler(2))(2, 0, 0); // Otherwise TwinBee will freeze after the second Reset
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Game);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =352;
} // namespace

MapperInfo MapperInfo_352 ={
	&MapperNum,
	_T("Reset-based NROM-256 (Mirroring dependent on game)"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};