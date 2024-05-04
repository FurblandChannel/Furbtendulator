#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg[8];

void	sync (void) {
	EMU->SetPRG_ROM8(0x8, reg[3]);
	EMU->SetPRG_ROM8(0xA, 0xFD);
	EMU->SetPRG_ROM8(0xC, 0xFE);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	EMU->SetCHR_ROM4(0, reg[0] >>2);
	EMU->SetCHR_ROM2(4, reg[1] >>1);
	EMU->SetCHR_ROM2(6, reg[2] >>1);
	if (reg[4] &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int, int addr, int val) {
	reg[addr &7] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	sync();
	
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =193;
} // namespace

MapperInfo MapperInfo_193 ={
	&mapperNum,
	_T("NTDEC 2394"),
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