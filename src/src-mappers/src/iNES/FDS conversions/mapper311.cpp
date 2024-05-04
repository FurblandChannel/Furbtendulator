#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg, Mirroring, IRQEnabled;
uint16_t IRQCount;
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync (void) {
	EMU->SetPRG_ROM4(0x5, 17);
	EMU->SetPRG_ROM8(0x6, 9);
	EMU->SetPRG_ROM32(0x8, Reg);
	iNES_SetCHR_Auto8(0, 0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_WORD(mode, offset, data, IRQCount);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

int	MAPINT	Read (int Bank, int Addr) {
	if (Addr < 0x20)
		return _Read4(Bank, Addr);
	else if (Addr >=0x42 && Addr <=0x55)
		return 0xFF;
	else
		return _Read4(Bank, Addr);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	switch (Addr) {
		case 0x22:
			Reg = Val &1;
			Sync();
			break;
		case 0x122:
			IRQEnabled =Val &1;
			IRQCount =0;
			EMU->SetIRQ(1);
			break;
		default:
			_Write4 (Bank, Addr, Val);
			break;
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	if (ResetType == RESET_HARD) Reg =IRQEnabled =IRQCount =0;

	_Read4 =EMU->GetCPUReadHandler(4);
	_Write4 =EMU->GetCPUWriteHandler(4);
	EMU->SetCPUReadHandler(4, Read);
	EMU->SetCPUWriteHandler(4, Write);
	Sync();
}

void	MAPINT	CPUCycle (void) {
	if (IRQEnabled) {
		if (IRQCount <4096)
			IRQCount++;
		else {
			IRQEnabled =0;
			EMU->SetIRQ(0);
		}
	}
}

uint16_t MapperNum =311;
} // namespace

MapperInfo MapperInfo_311 = {
	&MapperNum,
	_T("SMB2JX"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};