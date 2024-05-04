#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {

void	sync (void) {
	EMU->SetCHR_RAM8(0x0, 0);	
	EMU->SetPRG_ROM32(0x8, Latch::data);
	if (Latch::data &0x20) {
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
		if (Latch::data &0x10)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (Latch::data &0x20) {
		if (Latch::data &0x08)
			Latch::data =Latch::data &~0x17 | val &0x17;
		else
			Latch::data =Latch::data &~0x13 | val &0x13;
	} else
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

uint16_t mapperNum =564;
} // namespace

MapperInfo MapperInfo_564 = {
	&mapperNum,
	_T("bd23.pcb"),
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
