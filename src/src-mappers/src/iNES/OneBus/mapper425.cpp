#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
void	sync () {
	OneBus::syncPRG(0x0FFF, OneBus::reg4100[0xE6] <<12);
	OneBus::syncCHR(0x7FFF, OneBus::reg4100[0xE6] <<15);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reg4100[0xE6] =0;
	OneBus::reset(resetType);
}
uint16_t mapperNum =425;
} // namespace


MapperInfo MapperInfo_425 = {
	&mapperNum,
	_T("Cube Tech VT369"),
	COMPAT_FULL,
	load,
	reset,
	OneBus::unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};
