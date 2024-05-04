#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg       (Latch::addr >>5)
#define chr       (Latch::addr &0x03 | Latch::addr >>2 &0x04 | Latch::addr >>4 &0x08)
#define nrom    !!(Latch::addr &0x04)
#define mirrorH !!(Latch::addr &0x100)

namespace {
void	sync (void) {	
	if (nrom)
		EMU->SetPRG_ROM32(0x8, prg);
	else {
		EMU->SetPRG_ROM16(0x8, prg <<1);
		EMU->SetPRG_ROM16(0xC, prg <<1 | 7);
	}
	
	iNES_SetCHR_Auto8(0x0, chr);
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =459;
} // namespace

MapperInfo MapperInfo_459 ={
	&mapperNum,
	_T("8-in-1"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};