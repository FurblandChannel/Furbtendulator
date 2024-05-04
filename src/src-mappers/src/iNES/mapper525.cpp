#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint8_t		nt;
uint8_t		chr[8];

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg >>1);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	if (nt &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	writeNT (int bank, int addr, int val) {
	nt =val;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &7] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	sync();
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeNT);
	EMU->SetCPUWriteHandler(0xB, writeCHR);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, nt);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =525;
} // namespace

MapperInfo MapperInfo_525 = {
	&mapperNum,
	_T("KS-7021A"),
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