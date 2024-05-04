#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg                (Latch::addr >>3 &0x07)
#define chr                (Latch::addr >>1 &0x03 | Latch::addr >>2 &0x0C)
#define	nrom256             Latch::addr &0x20
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

uint16_t mapperNum =748;
} // namespace

MapperInfo MapperInfo_748 ={
	&mapperNum,
	_T("K-3069"), /* 1992 科技大震撼 孩子王 21-in-1 鑽石全卡 */
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
