#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_RAM4(0x7, 0);
	EMU->SetPRG_ROM16(0x8, reg[0] <<3 | Latch::data &7);
	EMU->SetPRG_ROM16(0xC, reg[0] <<3 |              7);
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[1] &1)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg[1] &0x80) {
		reg[addr &1] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x6, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =500;
} // namespace

MapperInfo MapperInfo_500 ={
	&mapperNum,
	_T("Yhc UNROM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};