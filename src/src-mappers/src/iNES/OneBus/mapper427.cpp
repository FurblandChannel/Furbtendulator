#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"
#include	"..\..\Hardware\h_EEPROM.h"

namespace {
OneBus::InverterROM  inverterROM;
EEPROM_24C04*        eeprom =NULL;

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	if (ROM->INES2_PRGRAM ==0x30) eeprom =new EEPROM_24C04(0, ROM->PRGRAMData);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reset(resetType);
	switch(ROM->INES2_SubMapper) {
		case 0:	// MOGIS M320, The Oregon Trail
			OneBus::gpio[1].attachSerialDevice({&inverterROM, 255, 5, 4 });
			if (eeprom) {
				eeprom->reset();
				OneBus::gpio[2].attachSerialDevice({eeprom, 255, 2, 3 });
			}
			break;
		case 1: // Lexibook Cyber Arcade Pocket
			OneBus::gpio[2].attachSerialDevice({&inverterROM, 255, 0, 1 });	
			break;
	}		
}

void	MAPINT	unload () {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

uint16_t mapperNum =427;
} // namespace


MapperInfo MapperInfo_427 = {
	&mapperNum,
	_T("VT369 with inverter ROM/EEPROM"),
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
