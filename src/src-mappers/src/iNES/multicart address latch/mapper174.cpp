#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg                (Latch::addr >>4 &0x07)
#define chr                (Latch::addr >>1 &0x07)
#define	nrom256             Latch::addr &0x80
#define horizontalMirroring Latch::addr &0x01

namespace {
void	sync (void) {
	if (nrom256)
		EMU->SetPRG_ROM32(0x8, prg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	}
	EMU->SetCHR_ROM8(0x0, chr);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =174;
} // namespace

MapperInfo MapperInfo_174 ={
	&mapperNum,
	_T("NTDEC 5-in-1"),
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
