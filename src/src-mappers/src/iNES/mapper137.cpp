#include "..\DLL\d_iNES.h"

namespace {
uint8_t		index;
uint8_t		reg[8];
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg[5]);
	iNES_SetCHR_Auto1(0x0, reg[reg[7] &1? 0: 0] &0x07                                      );
	iNES_SetCHR_Auto1(0x1, reg[reg[7] &1? 0: 1] &0x07                    | reg[4] <<4 &0x10);
	iNES_SetCHR_Auto1(0x2, reg[reg[7] &1? 0: 2] &0x07                    | reg[4] <<3 &0x10);
	iNES_SetCHR_Auto1(0x3, reg[reg[7] &1? 0: 3] &0x07 | reg[6] <<3 &0x08 | reg[4] <<2 &0x10);
	iNES_SetCHR_Auto4(0x4, 0xFF);
	switch (reg[7] &7) {
		default:EMU->Mirror_H();
			break;
		case 2:	EMU->Mirror_V();
			break;
		case 4:	EMU->Mirror_Custom(0, 0, 0, 1);
			break;
		case 6:	EMU->Mirror_S0();
			break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (bank &4 && addr &0x100) {
		if (addr &1) {
			reg[index &7] =val;
			sync();
		} else
			index =val;
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index =0;
		for (auto& c: reg) c =0;
	}
	sync();

	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank<=0xF; bank++) EMU->SetCPUWriteHandler (bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =137;
} // namespace

MapperInfo MapperInfo_137 ={
	&mapperNum,
	_T("聖謙 SA8259D"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};