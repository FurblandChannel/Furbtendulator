#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0, 0);
	EMU->SetPRG_ROM32(0x8, Reg);
}


void	MAPINT	Write (int Bank, int Addr, int Val) {
	switch (Addr) {
		case 0x0A0: Reg =0; Sync(); break;
		case 0xE36: Reg =1; Sync(); break;
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Reg =1;
	EMU->SetCPUWriteHandler(0xE, Write);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}


uint16_t MapperNum =346;
} // namespace

MapperInfo MapperInfo_346 ={
	&MapperNum,
	_T("Kaiser KS-7012"), // Zanac (Kaiser)
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
