#include	"..\..\DLL\d_iNES.h"

namespace {	
uint8_t	Mode, PRG[2], CurrentCHRBank;
FPPURead _ReadCHR;

void	Sync (void) {
	if (Mode &8)
		EMU->SetPRG_ROM32(0x8,((Mode <<3) | (PRG[CurrentCHRBank] &7)) >>1);
	else {
		EMU->SetPRG_ROM16(0x8, (Mode <<3) | (PRG[CurrentCHRBank] &7));
		EMU->SetPRG_ROM16(0xC, (Mode <<3) |                       7 );
	}
	EMU->SetCHR_ROM4(0, (Mode <<5) | (PRG[0] >>3));
	EMU->SetCHR_ROM4(4, (Mode <<5) | (PRG[1] >>3));
	if (Mode &4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	ReadCHR (int Bank, int Addr) {
	CurrentCHRBank =(Bank &4)? 1: 0;
	Sync();
	return _ReadCHR(Bank, Addr);
}

void	MAPINT	WritePRG0 (int Bank, int Addr, int Val) {
	PRG[0] =Val;
	Sync();
}

void	MAPINT	WritePRG1 (int Bank, int Addr, int Val) {
	PRG[1] =Val;
	Sync();
}

void	MAPINT	WriteMode (int Bank, int Addr, int Val) {
	Mode =Val &0xF;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) Mode =PRG[0] =PRG[1] =CurrentCHRBank =0;
	
	_ReadCHR =EMU->GetPPUReadHandler(0);
	for (int i =0; i <8; i++) {
		EMU->SetPPUReadHandler(i, ReadCHR);
		EMU->SetPPUReadHandlerDebug(i, ReadCHR);
	}
	for (int i =0xA; i<=0xB; i++) EMU->SetCPUWriteHandler(i, WritePRG0);
	for (int i =0xC; i<=0xD; i++) EMU->SetCPUWriteHandler(i, WritePRG1);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(0xE, WriteMode);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Mode);
	SAVELOAD_BYTE(mode, offset, data, PRG[0]);
	SAVELOAD_BYTE(mode, offset, data, PRG[1]);
	if (mode == STATE_LOAD) Sync();
	return offset;
}


uint16_t MapperNum =331;
} // namespace

MapperInfo MapperInfo_331 = {
	&MapperNum,
	_T("12-in-1"),
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