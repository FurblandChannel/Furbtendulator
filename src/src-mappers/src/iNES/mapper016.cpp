#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FCG.h"
#include	"..\Hardware\h_LZ93D50.h"

namespace {
EEPROM_I2C	*eeprom;

void	syncFCG (void) {
	FCG::syncPRG(0x0F, 0x00);
	FCG::syncCHR(0xFF, 0x00);
	FCG::syncMirror();
}

void	syncLZ93D50 (void) {
	LZ93D50::syncPRG(0x0F, 0x00);
	LZ93D50::syncCHR(0xFF, 0x00);
	LZ93D50::syncMirror();
}

BOOL	MAPINT	load (void) {
	eeprom =NULL;
	switch (ROM->INES2_SubMapper) {
		case 4:
			MapperInfo_016.Description = _T("Bandai FCG-1/2");
			MapperInfo_016.Reset =FCG::reset;
			MapperInfo_016.CPUCycle =FCG::cpuCycle;
			FCG::load(syncFCG);
			break;
		case 5:
			if (CART_BATTERY) {
				MapperInfo_016.Description = _T("Bandai LZ93D50 with 24C02 EEPROM");
				eeprom =new EEPROM_24C02(0, ROM->PRGRAMData);
			} else
				MapperInfo_016.Description = _T("Bandai LZ93D50");
			MapperInfo_016.Reset =LZ93D50::reset;
			MapperInfo_016.CPUCycle =LZ93D50::cpuCycle;
			LZ93D50::load(syncLZ93D50, eeprom, false);
			break;			
		default:			
			/* Emulate a "sloppy LZ93D50" that responds across the entire $6000-$FFFF CPU address range, but without EEPROM. */
			MapperInfo_016.Description = _T("Bandai FGC-1/2 or LZ93D50");
			MapperInfo_016.Reset =LZ93D50::reset;
			MapperInfo_016.CPUCycle =LZ93D50::cpuCycle;
			LZ93D50::load(syncLZ93D50, NULL, true);
			break;			
	}
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (ROM->INES2_SubMapper ==4) {
		offset =FCG::saveLoad(stateMode, offset, data);
		if (stateMode ==STATE_LOAD) syncFCG();
	} else {
		offset =LZ93D50::saveLoad(stateMode, offset, data);
		if (eeprom) offset =eeprom->saveLoad(stateMode, offset, data);	
		if (stateMode ==STATE_LOAD) syncLZ93D50();
	}
	return offset;
}

uint16_t mapperNum =16;
} // namespace

MapperInfo MapperInfo_016 ={
	&mapperNum,
	_T("Bandai FGC-1/2 or LZ93D50 with 24C02 EEPROM"),
	COMPAT_FULL,
	load,
	LZ93D50::reset,
	unload,
	LZ93D50::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};