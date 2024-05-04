#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::addr &2) {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>2);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>3);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &1)
		EMU->Mirror_V();
	else	
		EMU->Mirror_H();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <0xB; bank++) EMU->SetCPUWriteHandler(bank, (EMU->GetCPUWriteHandler(0x6))); // Some games act as if there is a CNROM switch there. But with only 8 KiB of CHR-RAM, that is impossible.
}

uint16_t MapperNum =541;
} // namespace

MapperInfo MapperInfo_541 ={
	&MapperNum,
	_T("LittleCom 160"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};