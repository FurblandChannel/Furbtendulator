#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define nrom256 !!(Latch::addr &0x40)
#define mirrorH !!(Latch::addr &0x20)
#define prg        Latch::addr >>7
#define chr        Latch::addr &0x1F

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =464;
} // namespace

MapperInfo MapperInfo_464 ={
	&mapperNum,
	_T("NTDEC 9012"),
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