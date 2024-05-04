#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	if (Latch::addr &0x800) { // UNROM
		EMU->SetPRG_ROM16(0x8, (Latch::addr &0x1F) | (Latch::addr &((Latch::addr &0x40)? 1: 0)) );
		EMU->SetPRG_ROM16(0xC, (Latch::addr &0x1F) | 7);
	} else
	if (Latch::addr &0x40) { // NROM-128
		EMU->SetPRG_ROM16(0x8, Latch::addr &0x1F);
		EMU->SetPRG_ROM16(0xC, Latch::addr &0x1F);
	} else // NROM-256
		EMU->SetPRG_ROM32(0x8, (Latch::addr &0x1F) >>1);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, TRUE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
}

uint16_t MapperNum =349;
} // namespace

MapperInfo MapperInfo_349 = {
	&MapperNum,
	_T("G-146"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};