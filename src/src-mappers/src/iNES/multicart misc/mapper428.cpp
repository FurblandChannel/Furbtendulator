#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[4];

void	sync (void) {
	if (reg[1] &0x10)
		EMU->SetPRG_ROM32(0x8, reg[1] >>6);
	else {
		EMU->SetPRG_ROM16(0x8, reg[1] >>5);
		EMU->SetPRG_ROM16(0xC, reg[1] >>5);
	}
	EMU->SetCHR_ROM8(0, reg[1] &0x7 &~(reg[2] >>6) | Latch::data &(reg[2] >>6));
	
	if (reg[1] &0x08)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue &3 | *EMU->OpenBus &~3;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &3] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	for (int bank =0x6; bank <=0x7; bank++) {
		EMU->SetCPUReadHandler(bank, readDIP);
		EMU->SetCPUWriteHandler(bank, writeReg);
	}
	Latch::reset(resetType);
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =428;
} // namespace

MapperInfo MapperInfo_428 ={
	&mapperNum,
	_T("BB-002A/TF2740"),
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
