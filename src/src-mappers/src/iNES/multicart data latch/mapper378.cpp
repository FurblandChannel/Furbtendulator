#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, ((Latch::data &7) <<1) |((Latch::data &8) >>3)           | ((Latch::data &0x20) >>1));
	EMU->SetPRG_ROM16(0xC, ((Latch::data &7) <<1) | (Latch::data &0x20? 0x07: 0x01) | ((Latch::data &0x20) >>1));
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x20) {
		if (Latch::data &0x04)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	} else {
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	}
}

BOOL	MAPINT load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	sync();
}

uint16_t mapperNum =378;
} // namespace

MapperInfo MapperInfo_378 ={
	&mapperNum,
	_T("8-in-1 AOROM+UNROM"),
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
