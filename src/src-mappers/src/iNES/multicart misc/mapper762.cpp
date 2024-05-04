#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::addr &0x20) {
		EMU->SetPRG_ROM16(0x8, 0x08 +(Latch::addr >>1 &8 | Latch::data &7));
		EMU->SetPRG_ROM16(0xC, 0x08 +(Latch::addr >>1 &8 |              7));
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1 &7);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1 &7);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (Latch::data &8 && ~val &8) Latch::addr =addr;
	Latch::data =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}


uint16_t MapperNum =762;
} // namespace

MapperInfo MapperInfo_762 = { /* PRELIMINARY mapper number */
	&MapperNum,
	_T("810706"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};