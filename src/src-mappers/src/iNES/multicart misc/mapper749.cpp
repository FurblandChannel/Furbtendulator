#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	if (reg[0] &0x20)
		EMU->SetPRG_ROM32(0x8, reg[0] >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg[0] &0x1F);
		EMU->SetPRG_ROM16(0xC, reg[0] &0x1F);
	}
	EMU->SetCHR_ROM8(0x0, reg[1] &0x1F);
	if (reg[1] &0x20)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==4) EMU->WriteAPU(bank, addr, val);
	if (addr &0x100) {
		reg[bank >>1 &1] =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();
	for (int bank =0x4; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =749;
} // namespace

MapperInfo MapperInfo_749 = {
	&mapperNum,
	_T("ST-32"), /* Super 32-in-1 */
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