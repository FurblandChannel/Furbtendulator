#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg, NewReg, Mirroring;
FCPURead _ReadF;

void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Reg);
	EMU->SetPRG_ROM16(0xC, Reg);
	EMU->SetCHR_ROM8(0, Reg);
	if (Mirroring &4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	ReadReset (int Bank, int Addr) {
	if (Reg !=NewReg) {
		Reg =NewReg;
		Sync();
	}
	return _ReadF(Bank, Addr);
}

void	MAPINT	WriteMirroring (int Bank, int Addr, int Val) {
	Mirroring =Val;
	Sync();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	NewReg =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	_ReadF =EMU->GetCPUReadHandler(0xF);
	EMU->SetCPUWriteHandler(0x8, WriteMirroring);
	EMU->SetCPUWriteHandler(0xA, WriteReg);
	EMU->SetCPUReadHandler(0xF, ReadReset);
	if (ResetType ==RESET_HARD) Reg =NewReg =Mirroring =0;
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	SAVELOAD_BYTE(mode, offset, data, NewReg);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =175;
} // namespace

MapperInfo MapperInfo_175 ={
	&MapperNum,
	_T("Kaiser KS-122"),
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
