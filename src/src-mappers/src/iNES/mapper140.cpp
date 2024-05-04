#include	"..\DLL\d_iNES.h"

namespace {
uint8_t Reg;

void	Sync (void) {
	EMU->SetPRG_ROM32(0x8, Reg >>4);
	EMU->SetCHR_ROM8(0, Reg &0xF);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Reg =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	EMU->SetCPUWriteHandler(0x6, Write);
	EMU->SetCPUWriteHandler(0x7, Write);
	if (ResetType == RESET_HARD) Reg =0;
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =140;
} // namespace

MapperInfo MapperInfo_140 =
{
	&MapperNum,
	_T("Jaleco GNROM"),
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