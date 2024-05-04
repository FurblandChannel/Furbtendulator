#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
void	sync () { 
	if (ROM->INES2_SubMapper ==1) {
		OneBus::syncPRG(0xF7FF, OneBus::reg4100[0x1C] &0x01 && ~OneBus::reg4100[0x1C] &0x04? 0x0800: 0x0000);
		OneBus::syncCHR(0xBFFF, OneBus::reg4100[0x1C] &0x01 && ~OneBus::reg4100[0x1C] &0x04? 0x4000: 0x0000);
	} else {
		OneBus::syncPRG(0xF3FF, (OneBus::reg4100[0x0F] &0x20? 0x0400: 0x0000) | (OneBus::reg4100[0x00] &0x40? 0x0800:0x000));
		OneBus::syncCHR(0x9FFF, (OneBus::reg4100[0x0F] &0x20? 0x2000: 0x0000) | (OneBus::reg4100[0x00] &0x04? 0x4000:0x000));
	}
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	OneBus::load(sync);
	return TRUE;
}

uint16_t mapperNum =436;
} // namespace

MapperInfo MapperInfo_436 ={
	&mapperNum,
	_T("ZLX-08"),
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