#include	"..\DLL\d_iNES.h"

namespace NINA001 {
uint8_t		reg[3];
FCPUWrite	writeWRAM;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, reg[0]);
	EMU->SetCHR_ROM4(0, reg[1]);
	EMU->SetCHR_ROM4(4, reg[2]);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeWRAM(bank, addr, val);
	if (addr >=0xFFD) {
		reg[addr -0xFFD] =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	sync();
	writeWRAM =EMU->GetCPUWriteHandler(0x7);
	EMU->SetCPUWriteHandler(0x7, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =34;

MapperInfo MapperInfo_NINA001 ={
	&mapperNum,
	_T("AVE NINA-001"),
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
} // namespace
