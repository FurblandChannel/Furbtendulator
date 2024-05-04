#include "..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg8, RegC;

void	Sync (void) {
	if (RegC &0x10) {
		EMU->SetPRG_ROM16(0x8, ((RegC &0x07)<<1) | ((RegC &0x08)? 1: 0));
		EMU->SetPRG_ROM16(0xC, ((RegC &0x07)<<1) | ((RegC &0x08)? 1: 0));
	} else
		EMU->SetPRG_ROM32(0x8, RegC &0x07);
	EMU->SetCHR_ROM8(0x0, Reg8);
	if (RegC &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	Write8 (int Bank, int Addr, int Val) {
	Reg8 =Val;
	Sync();
}

void	MAPINT	WriteC (int Bank, int Addr, int Val) {
	RegC =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0x8; i<=0xB; i++) EMU->SetCPUWriteHandler(i, Write8);
	for (int i =0xC; i<=0xF; i++) EMU->SetCPUWriteHandler(i, WriteC);
	if (ResetType ==RESET_HARD) Reg8 =RegC =0;
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg8);
	SAVELOAD_BYTE(mode, offset, data, RegC);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum = 335;
} // namespace

MapperInfo MapperInfo_335 = {
	&MapperNum,
	_T("CTC-09"),
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
