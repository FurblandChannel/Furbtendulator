#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	Reg;

int	MAPINT	Read (int Bank, int Addr) {
	if (Addr ==0x001 || Addr ==0x777)
		return Reg | (*EMU->OpenBus &0x7F);
	else
		return *EMU->OpenBus;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if ((Bank ==0x6 && Addr ==0x502) || (Bank ==0x7 && Addr ==0x000))
		Reg =(Val <<1) &0x80;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) Reg =0;
	iNES_SetMirroring();
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, 0);
	EMU->SetCPUReadHandler(7, Read);
	EMU->SetCPUWriteHandler(6, Write);
	EMU->SetCPUWriteHandler(7, Write);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	return offset;
}

uint16_t MapperNum =170;
} // namespace

MapperInfo MapperInfo_170 ={
	&MapperNum,
	_T("藤屋 NROM"),
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