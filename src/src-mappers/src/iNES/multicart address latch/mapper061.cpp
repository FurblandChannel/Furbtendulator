#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg                (Latch::addr &0x0F)
#define prgA14             (Latch::addr >>5 &1)
#define nrom128             Latch::addr &0x10
#define	horizontalMirroring Latch::addr &0x80
namespace {
void	sync (void) {
	if (nrom128) {
		EMU->SetPRG_ROM16(0x8, prg <<1 | prgA14);
		EMU->SetPRG_ROM16(0xC, prg <<1 | prgA14);
	} else
		EMU->SetPRG_ROM32(0x8, prg);
	
	EMU->SetCHR_RAM8(0x0, 0);
	
	if (horizontalMirroring)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =61;
} // namespace

MapperInfo MapperInfo_061 ={
	&mapperNum,
	_T("GS-2017"),
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