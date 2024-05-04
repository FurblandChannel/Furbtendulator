#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	PRG, CHR;

void	Sync (void) {
	EMU->SetPRG_ROM32(8, PRG);
	EMU->SetCHR_ROM8(0, CHR);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	SAVELOAD_BYTE(mode, offset, data, CHR);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

static const uint8_t LUT_PRG [4][4] ={
	{ 0, 1, 2, 3, },
	{ 3, 2, 1, 0, },
	{ 0, 2, 1, 3, },
	{ 3, 1, 2, 0, },
};

static const uint8_t LUT_CHR [8][8] ={
	{ 0, 1, 2, 3, 4, 5, 6, 7, },
	{ 0, 2, 1, 3, 4, 6, 5, 7, },
	{ 0, 1, 4, 5, 2, 3, 6, 7, },
	{ 0, 4, 1, 5, 2, 6, 3, 7, },
	{ 0, 4, 2, 6, 1, 5, 3, 7, },
	{ 0, 2, 4, 6, 1, 3, 5, 7, },
	{ 7, 6, 5, 4, 3, 2, 1, 0, },
	{ 7, 6, 5, 4, 3, 2, 1, 0, },
};

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Val &8)
		CHR =LUT_CHR[(Val >>4) &7][Val &7];
	else
		PRG =LUT_PRG[(Val >>4) &3][Val &3];	
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
	if (ResetType == RESET_HARD) PRG =CHR =0;
	Sync();
}

uint16_t MapperNum = 244;
} // namespace

MapperInfo MapperInfo_244 = {
	&MapperNum,
	_T("C&E Decathlon"),
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