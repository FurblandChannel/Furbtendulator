#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
void	sync () { 
	OneBus::syncPRG(0x0FFF, 0);
	if (OneBus::reg4100[0x00] ==0x6A) {
		EMU->SetCHR_RAM8(0x00, 0); // CPU
		EMU->SetCHR_RAM8(0x20, 0); // BG
		EMU->SetCHR_RAM8(0x28, 0); // SPR
	} else
		OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();	
}

BOOL	MAPINT	load (void) {
	OneBus::load(sync);
	return TRUE;
}

uint16_t mapperNum =408;
} // namespace

MapperInfo MapperInfo_408 = {
	&mapperNum,
	_T("Konami Collector's Series: Arcade Advanced"),
	COMPAT_FULL,
	load,
	OneBus::reset,
	OneBus::unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};