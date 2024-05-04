#include "..\DLL\d_iNES.h"

namespace {
uint8_t	*WRAM, Reg[4];
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, Reg[0] >>6);
	EMU->SetPRG_ROM16(0x8, Reg[1]);
	EMU->SetPRG_ROM16(0xC, 0);
	EMU->SetCHR_RAM8(0, 0);
}

int	MAPINT	Read4 (int Bank, int Addr) {
	if (Addr ==0x202)
		return 0x40;
	else if (Addr >=0x200 && Addr <=0x203)
		return 0x00;
	else if (Addr >=0x400)
		return WRAM[Addr -0x400];
	else
		return _Read4(Bank, Addr);
}

void	MAPINT	Write4 (int Bank, int Addr, int Val) {
	_Write4(Bank, Addr, Val);
	if (Addr >=0x200 && Addr <=0x203) {
		Reg[Addr &3] =Val;
		Sync();
	} else if (Addr >=0x400)
		WRAM[Addr -0x400] =Val;
}

BOOL	MAPINT	Load (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	WRAM =EMU->GetPRG_Ptr4(0x6);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) for (int i =0; i <4; i++) Reg[i] =0;
	_Read4 =EMU->GetCPUReadHandler(0x4);
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUWriteHandler(0x4, Write4);
	iNES_SetMirroring();
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, Reg[i]);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =186;
} // namespace

MapperInfo MapperInfo_186 ={
	&MapperNum,
	_T("Family Study Box by Fukutake Shoten"),
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
