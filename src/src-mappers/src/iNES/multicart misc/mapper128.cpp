#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint16_t	outerBank;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | outerBank >>2);
	EMU->SetPRG_ROM16(0xC,              7 | outerBank >>2);
	EMU->SetCHR_RAM8(0, 0);
	if (outerBank &0x02)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeOuterBank(int bank, int addr, int val) {
	if (outerBank <0xF000) outerBank =bank <<12 |addr;
	Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_WORD(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =128;
} // namespace

MapperInfo MapperInfo_128 = {
	&mapperNum,
	_T("4-in-1"),
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