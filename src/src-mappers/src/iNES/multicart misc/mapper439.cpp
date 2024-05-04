#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, reg[0] >>1 &~7 | Latch::data &7);
	EMU->SetPRG_ROM16(0xC, reg[0] >>1 &~7 |              7);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::data &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (~reg[0] &0x80)
		Latch::data =Latch::data &~0x07 | val & 0x07;
	else
		Latch::data =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0xFF;
	Latch::reset(RESET_HARD);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =439;
} // namespace

MapperInfo MapperInfo_439 ={
	&mapperNum,
	_T("YS2309"),
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