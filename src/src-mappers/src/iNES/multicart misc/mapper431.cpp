#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		outer;
uint8_t		inner;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, outer >>2 &~7 | inner &7);
	EMU->SetPRG_ROM16(0xC, outer >>2 &~7 |        7);
	iNES_SetCHR_Auto8(0x0, 0);
	if (outer &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT writeOuter (int bank, int addr, int val) {
	outer =val;
	sync();
}

void MAPINT writeInner (int bank, int addr, int val) {
	inner =val;
	sync();
}

void MAPINT reset (RESET_TYPE resetType) {
	outer =0;
	inner =0;
	sync();
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuter);
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeInner);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outer);
	SAVELOAD_BYTE(stateMode, offset, data, inner);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =431;
} // namespace

MapperInfo MapperInfo_431 ={
	&mapperNum,
	_T("Realtec GN-91B"),
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