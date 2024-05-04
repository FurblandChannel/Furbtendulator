#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define chr       (Latch::addr >>8)
#define nrom128 !!(Latch::addr &0x10)
#define prg       (Latch::addr <<1 | !!(Latch::addr &0x20))
#define mirrorH !!(Latch::addr &0x80)
namespace {
void	sync (void) {
	if (nrom128) 	{
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	} else
		EMU->SetPRG_ROM32(0x8, prg >>1);

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

uint16_t mapperNum =461;
} // namespace

MapperInfo MapperInfo_461 ={
	&mapperNum,
	_T("0324"),
	COMPAT_FULL,
	load,
	Latch::reset, // Does *not* reset to menu; using resetHard would lead to corrupt Galaxian after reset
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};
