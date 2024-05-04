#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, reg[1] &7 |8);
	EMU->SetPRG_ROM32(0x8, reg[0] >>4 &7);
	iNES_SetCHR_Auto8(0x0, reg[0] &0xF);
	if (reg[0] &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[bank &1] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();
	for (int bank =0xE; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =744;
} // namespace

MapperInfo MapperInfo_744 = {
	&mapperNum,
	_T("ET-81"), /* 超級巨王 24-in-1 高K 鑽石組 */
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
