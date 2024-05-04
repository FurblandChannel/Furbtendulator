#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	if (Latch::addr &0x010) {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<1 &0x1E | (Latch::addr &0x020? 1: 0));
		EMU->SetPRG_ROM16(0xC, Latch::addr <<1 &0x1E | (Latch::addr &0x020? 1: 0));
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr &0xF);
	EMU->SetCHR_ROM8(0, Latch::addr >>6 &1 | Latch::addr >>7 &~1);
	if (Latch::addr &0x0080)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, TRUE);
	return TRUE;
}

uint16_t MapperNum =763;
} // namespace

MapperInfo MapperInfo_763 = { /* PRELIMINARY mapper number */
	&MapperNum,
	_T("BS-N032"),
	COMPAT_FULL,
	Load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};