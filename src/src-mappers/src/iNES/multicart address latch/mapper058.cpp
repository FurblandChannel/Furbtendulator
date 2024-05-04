#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg     (Latch::addr &0x07)
#define chr     (Latch::addr >>3 &0x07)
#define	cpuA14 !(Latch::addr &0x40)
#define mirrorH  Latch::addr &0x80

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
	EMU->SetPRG_ROM16(0xC, prg | cpuA14);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum058 =58;
uint16_t mapperNum213 =213;
} // namespace

MapperInfo MapperInfo_058 ={
	&mapperNum058,
	_T("GK-192"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};
MapperInfo MapperInfo_213 ={
	&mapperNum213,
	_T("GK-192"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};
