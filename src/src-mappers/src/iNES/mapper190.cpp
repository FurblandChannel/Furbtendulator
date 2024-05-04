#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	PRG;
uint16_t	CHR[4];

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, PRG);
	EMU->SetPRG_ROM16(0xC, 0);
	for (int i =0; i <4; i++) EMU->SetCHR_ROM2(i <<1, CHR[i]);
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	return TRUE;
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	if (mode == STATE_LOAD)	Sync();
	return offset;
}

void	MAPINT	WritePRG(int Bank, int Addr, int Val) {
	PRG = (Val &7) + ((Bank >=0x0C)? 8: 0);
	Sync();
}

void	MAPINT	WriteCHR(int Bank, int Addr, int Val) {
	CHR[Addr &3] =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	EMU->SetCPUWriteHandler(0x8, WritePRG);
	EMU->SetCPUWriteHandler(0x9, WritePRG);
	EMU->SetCPUWriteHandler(0xA, WriteCHR);
	EMU->SetCPUWriteHandler(0xB, WriteCHR);
	EMU->SetCPUWriteHandler(0xC, WritePRG);
	EMU->SetCPUWriteHandler(0xD, WritePRG);
	if (ResetType ==RESET_HARD) {
		PRG =0;
		for (int i =0; i <4; i++) CHR[i] =0;
	}
	Sync();
}

uint16_t MapperNum = 190;
} // namespace

MapperInfo MapperInfo_190 =
{
	&MapperNum,
	_T("Zemina"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};