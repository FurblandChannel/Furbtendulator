#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14   !(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define nrom    !!(Latch::addr &0x200)
#define protect !!(Latch::addr &0x200)
#define prg       (Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20 | Latch::addr >>4 &0x40)

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~(cpuA14*nrom)        );
	EMU->SetPRG_ROM16(0xC, prg |  cpuA14*nrom |7*!nrom);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (protect) protectCHRRAM();
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();	
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =435;
} // namespace


MapperInfo MapperInfo_435 = {
	&mapperNum,
	_T("F-1002"),
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