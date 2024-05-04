#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define innerCHR           (Latch::data &0x03)
#define outerCHR           (Latch::addr &0x1F)
#define nrom128             Latch::addr &0x20
#define prg                (Latch::addr >>8 &0x3F | Latch::addr &0x40)
#define horizontalMirroring Latch::addr &0x80
namespace {
void	sync (void) {
	if (nrom128) 	{
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	} else
		EMU->SetPRG_ROM32(0x8, prg >>1);

	EMU->SetCHR_ROM8(0x0, outerCHR <<2 | innerCHR);
	
	if (horizontalMirroring)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =62;
} // namespace

MapperInfo MapperInfo_062 ={
	&mapperNum,
	_T("K-1016/N-190B"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};
