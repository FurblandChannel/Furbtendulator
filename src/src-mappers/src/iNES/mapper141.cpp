#include "..\DLL\d_iNES.h"

namespace {
uint8_t		index;
uint8_t		reg[8];
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg[5]);
	if (ROM->CHRROMSize) {
		EMU->SetCHR_ROM2(0x0, (reg[reg[7] &1? 0: 0] &0x07 | reg[4] <<3) <<1 |0);
		EMU->SetCHR_ROM2(0x2, (reg[reg[7] &1? 0: 1] &0x07 | reg[4] <<3) <<1 |1);
		EMU->SetCHR_ROM2(0x4, (reg[reg[7] &1? 0: 2] &0x07 | reg[4] <<3) <<1 |0);
		EMU->SetCHR_ROM2(0x6, (reg[reg[7] &1? 0: 3] &0x07 | reg[4] <<3) <<1 |1);
	} else
		EMU->SetCHR_RAM8(0x0, 0);
	
	switch (reg[7] &7) {
		case 0:	EMU->Mirror_Custom(0, 0, 0, 1);
			break;
		case 2:	EMU->Mirror_H();
			break;
		default:EMU->Mirror_V();
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

uint16_t mapperNum135 =135;
uint16_t mapperNum141 =141;
} // namespace

MapperInfo MapperInfo_135 ={
	&mapperNum135,
	_T("聖謙 TC-021A"),
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
MapperInfo MapperInfo_141 ={
	&mapperNum141,
	_T("聖謙 2M-RAM-COB"),
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