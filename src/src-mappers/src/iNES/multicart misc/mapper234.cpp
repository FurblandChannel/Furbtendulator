#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg[3];

void	sync (void) {
	int prg, chr;
	if (reg[0] &0x40) { // NINA-03
		prg =reg[0] &0x0E | reg[2] &0x01;
		chr =reg[0] <<2 &0x38 | reg[2] >>4 &0x07;
	} else { // CNROM
		prg =reg[0] &0x0F;
		chr =reg[0] <<2 &0x3C | reg[2] >>4 &0x03;
	}
	if (~reg[0] &0x10) {
		prg |=reg[0] >>1 &0x10;
		chr |=reg[0] <<1 &0x40;
	}
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_ROM8(0x0, chr);
	if (reg[0] &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readReg (int bank, int addr) {
	int result =EMU->ReadPRG(bank, addr);
	if (addr >=0xFE8 && addr <0xFF8) {
		reg[2] =result;
		sync();
	} else
	if (addr >=0xFC0 && addr <0xFE0 && !(reg[0] &0x3F)) {
		reg[1] =result;
		sync();
	} else
	if (addr >=0xF80 && addr <0xFA0 && !(reg[0] &0x3F)) {
		reg[0] =result;
		sync();
	}
	return result;
}

void	MAPINT	reset (RESET_TYPE) {
	for (auto& c: reg) c =0;
	sync();
	EMU->SetCPUReadHandler(0xF, readReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =234;
} // namespace

MapperInfo MapperInfo_234 = {
	&mapperNum,
	_T("Maxi 15"),
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