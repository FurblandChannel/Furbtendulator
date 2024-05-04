#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg[3];

void	sync (void) {
	EMU->SetCHR_RAM8(0, reg[2]);
	int prg =reg[0] &0x3F | reg[1] <<4 &~0x3F;
	switch(reg[1] &3) {
		case 0: EMU->SetPRG_ROM32(0x8, prg >>1);
			protectCHRRAM();
			break;		
		case 1:	EMU->SetPRG_ROM16(0x8, prg   );
			EMU->SetPRG_ROM16(0xC, prg |7);
			break;
		case 2: EMU->SetPRG_ROM8 (0x8, prg <<1 | reg[0] >>7);
			EMU->SetPRG_ROM8 (0xA, prg <<1 | reg[0] >>7);
			EMU->SetPRG_ROM8 (0xC, prg <<1 | reg[0] >>7);
			EMU->SetPRG_ROM8 (0xE, prg <<1 | reg[0] >>7);
			break;
		case 3:	EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
			protectCHRRAM();
			break;
	}
	if (reg[0] &0x40)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg0 (int bank, int addr, int val) {
	reg[0] =val;
	sync();
}

void	MAPINT	writeReg1 (int bank, int addr, int val) {
	reg[1] =addr &0xFF;
	reg[2] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0x00;
	sync();
	for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeReg0);
	for (int bank =0xC; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =310;
} // namespace

MapperInfo MapperInfo_310 = {
	&mapperNum,
	_T("K-1053"),
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