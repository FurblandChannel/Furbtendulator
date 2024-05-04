#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | reg <<3);
	EMU->SetPRG_ROM16(0xC,              7 | reg <<3);
	iNES_SetCHR_Auto8(0x0, 0);
	if (reg &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	Latch::reset(RESET_HARD);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =294;
} // namespace

MapperInfo MapperInfo_294 ={
	&mapperNum,
	_T("63-1601"),
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