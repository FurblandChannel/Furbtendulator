#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
OneBus::SerialROM* serialROM =NULL;

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	if (ROM->MiscROMSize &0x0100) serialROM =new OneBus::SerialROM(ROM->MiscROMData +ROM->MiscROMSize -0x0100);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reset(resetType);
	if (serialROM) {
		serialROM->reset();
		OneBus::gpio[2].attachSerialDevice({ serialROM, 4, 3, 5 });
	}
}

void	MAPINT	unload () {
	if (serialROM) {
		delete serialROM;
		serialROM =NULL;
	}
}
uint16_t mapperNum =426;
} // namespace


MapperInfo MapperInfo_426 = {
	&mapperNum,
	_T("VT369 with serial ROM"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};
