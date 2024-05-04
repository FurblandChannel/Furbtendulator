#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
//#define prg       (Latch::data &0x1F)
#define prg       (Latch::data >>1 &0x18 | Latch::data &0x07)
#define mirrorH !!(Latch::data &0x20)
#define nrom256 !!(Latch::data &0x40)
#define aorom   !!(Latch::data &0x80)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256 |!nrom256*7);
	EMU->SetCHR_RAM8(0, 0);

	if (aorom) {
		if (mirrorH)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		if (mirrorH)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

BOOL	MAPINT	load (void) {	
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =285;
} // namespace

MapperInfo MapperInfo_285 = {
	&mapperNum,
	_T("A65AS"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
