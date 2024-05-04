#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg           (Latch::addr &0x1F)
#define	nrom128    !!(Latch::addr &0x40)
#define mirrorH    !!(Latch::addr &0x80)
#define protectCHR  !(Latch::addr &0x400)
#define fds        !!(Latch::addr &0x800)

namespace {
void	sync (void) {
	if (fds)
		EMU->SetPRG_ROM8(0x6, prg <<1 |3);
	else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
	if (nrom128) {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	} else
		EMU->SetPRG_ROM32(0x8, prg >>1);
	EMU->SetCHR_RAM8(0x0, 0);
	if (protectCHR) protectCHRRAM();

	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =402;
} // namespace

MapperInfo MapperInfo_402 ={
	&mapperNum,
	_T("J-2282"),
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
