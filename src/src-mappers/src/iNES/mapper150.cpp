#include "..\DLL\d_iNES.h"

namespace {
uint8_t		index;
uint8_t		reg[8];
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg[2] &1 | reg[5]);
	
	if (ROM->INES_MapperNum ==243)  // SA-020A
		EMU->SetCHR_ROM8(0, reg[2] &1 | reg[4] <<1 &2 | reg[6] <<2);
	else                            // SA-015
		EMU->SetCHR_ROM8(0, reg[6] &3 | reg[4] <<2 &4 | reg[2] <<3);

	switch (reg[7] &6) {
		case 0: EMU->Mirror_Custom(0, 0, 0, 1);
			break;
		case 2: EMU->Mirror_H();
			break;
		case 4:	EMU->Mirror_V();
			break;
		case 6:	EMU->Mirror_S1();
			break;
	}
}

int	MAPINT	readASIC (int bank, int addr) {
	if ((addr &0x101) ==0x101)
		return reg[index] &(0x07 &~ROM->dipValue) | *EMU->OpenBus &~(0x07 &~ROM->dipValue);
	else
		return readAPU(bank, addr);
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		val =val &7 |ROM->dipValue;
		if (addr &1) {
			reg[index &7] =val;
			sync();
		} else
			index =val;
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index =ROM->dipValue;
		for (auto& c: reg) c =0;
	}
	sync();

	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank<=0x5; bank++) {
		EMU->SetCPUReadHandler     (bank, readASIC);
		EMU->SetCPUReadHandlerDebug(bank, readASIC);
		EMU->SetCPUWriteHandler    (bank, writeASIC);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum150 =150;
uint16_t mapperNum243 =243;
} // namespace

MapperInfo MapperInfo_150 ={
	&mapperNum150,
	_T("聖謙 SA-015/SA-630"),
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
MapperInfo MapperInfo_243 ={
	&mapperNum243,
	_T("聖謙 SA-020A"),
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