#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		shift;
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, reg);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr ==0) {
		if (~shift &4 && val &4) {
			reg =reg >>1 | val <<2 &4;
			reg &=7;
			sync();
		}
		shift =val;
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	EMU->SetCPUWriteHandler(0xC, writeReg);
	sync();
}

uint16_t mapperNum =756;
} // namespace

MapperInfo MapperInfo_756={
	&mapperNum,
	_T("Pro Action Rocky"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};