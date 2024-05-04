#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t PRG;
FCPUWrite _Write4;

void	Sync() {
	EMU->SetPRG_ROM8(6, PRG);
	EMU->SetPRG_ROM32(8, 2);
	iNES_SetCHR_Auto8(0, 0);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	_Write4(Bank, Addr, Val);
	if (Addr ==0x1FF) {
		PRG =Val;
		Sync();
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	if (ResetType ==RESET_HARD) PRG =0;
	
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, Write);
	EMU->SetCPUWriteHandler(0x5, Write);

	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =120;
} // namespace

MapperInfo MapperInfo_120 ={
	&MapperNum,
	_T("FDS Tobidase Daisakusen"),
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