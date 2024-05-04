#include	"..\DLL\d_iNES.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, reg >>2);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr ==0x016) {
		reg =val;
		sync();
	}
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

uint16_t mapperNum =99;
} // namespace

MapperInfo MapperInfo_099 ={
	&mapperNum,
	_T("Nintendo Vs. System"),
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
