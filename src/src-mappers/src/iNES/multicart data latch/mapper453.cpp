#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::data &0x40) {
		EMU->SetPRG_ROM32(0x8, Latch::data &7 | Latch::data >>3 &0x18);
		if (Latch::data &0x10)
			EMU->Mirror_S0();
		else
			EMU->Mirror_S1();
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::data &7 | Latch::data >>2 &0x38);
		EMU->SetPRG_ROM16(0xC,              7 | Latch::data >>2 &0x38);
		if (Latch::data &0x10)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
	EMU->SetCHR_RAM8(0x0, 0x0);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (Latch::data &0xE0)
		Latch::data =Latch::data &0xE0 |val &~0xE0;
	else
		Latch::data =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =453;
} // namespace

MapperInfo MapperInfo_453 ={
	&mapperNum,
	_T("Realtec 8042"),
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