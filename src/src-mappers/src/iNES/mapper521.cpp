#include	"..\DLL\d_iNES.h"
//장두진 바둑교실 - 임운편
namespace {
uint8_t	PRG;

void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, PRG);
	EMU->SetPRG_ROM16(0xC, 0x8);
	EMU->SetCHR_RAM8(0, 0);
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	PRG =Val;
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	EMU->SetCPUWriteHandler(5, WritePRG);
	Sync();
}

uint16_t MapperNum =521;
} // namespace

MapperInfo MapperInfo_521 = {
	&MapperNum,
	_T("Dreamtech 01"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};