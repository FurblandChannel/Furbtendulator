#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg[2];	

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM4(0, reg[0]);
	EMU->SetCHR_ROM4(4, reg[1]);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	sync();

	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =171;
} // namespace

MapperInfo MapperInfo_171 ={
	&mapperNum,
	_T("Kaiser KS-7058"),
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
