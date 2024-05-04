#include "..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg2, Reg1;

void	Sync (void) {
	uint8_t	Mode =((Reg1 &0x08)? 2: 0) | ((Reg2 &0x40)? 1: 0);
	uint8_t	OuterBank = ((Reg2 &0x01)? 0x20: 0x00) | ((Reg2 &0x20)? 0x10: 0x00) | ((Reg2 &0x10)? 0x08: 0x00);
	uint8_t	InnerBank =Reg1 &0x07;
	
	switch (Mode) {
		case 0:	EMU->SetPRG_ROM16(0x8, OuterBank |InnerBank);
			EMU->SetPRG_ROM16(0xC, OuterBank |7);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, OuterBank |InnerBank &~1);
			EMU->SetPRG_ROM16(0xC, OuterBank |7);
			break;
		case 2:	EMU->SetPRG_ROM16(0x8, OuterBank |InnerBank);
			EMU->SetPRG_ROM16(0xC, OuterBank |InnerBank);
			break;
		case 3:	EMU->SetPRG_ROM32(0x8, (OuterBank |InnerBank) >>1);
			break;
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Reg2 &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg2);
	SAVELOAD_BYTE(mode, offset, data, Reg1);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (~Bank &2) Reg1 =Val;
	if (~Bank &4) Reg2 =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0; i <8; i++) EMU->SetCPUWriteHandler(0x8 +i, Write);
	if (ResetType ==RESET_HARD) Reg1 =Reg2 =0;
	Sync();
}

uint16_t MapperNum = 293;
} // namespace

MapperInfo MapperInfo_293 = {
	&MapperNum,
	_T("BMC NEWSTAR 12-IN-1/76-IN-1"),
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
