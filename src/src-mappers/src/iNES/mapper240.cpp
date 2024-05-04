#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg >>4);
	EMU->SetCHR_ROM8(0x0, reg &0xF);
	EMU->SetPRG_RAM8(0x6, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr >0x20) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load () {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	sync();

	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =240;
} // namespace

MapperInfo MapperInfo_240 = {
	&mapperNum,
	_T("C&E Shenghuo Liezhuan"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};