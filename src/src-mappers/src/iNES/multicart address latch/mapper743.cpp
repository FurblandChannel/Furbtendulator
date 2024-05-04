#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::addr &8) {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	EMU->SetCHR_ROM8(0, Latch::addr);
	if (Latch::addr &8)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t mapperNum =743;
} // namespace

MapperInfo MapperInfo_743 ={
	&mapperNum,
	_T("HN-02"), /* 超級黃金版槍卡 1992 益智版 - Super Gun Game 36-in-1 */
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