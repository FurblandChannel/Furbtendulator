#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_LZ93D50.h"

namespace {
EEPROM_I2C	*eeprom =NULL;

void	sync (void) {
	LZ93D50::syncPRG(0x1F, 0x00); // Should be 0F, but there are fan translations that use 512 KiB
	LZ93D50::syncCHR(0xFF, 0x00);
	LZ93D50::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->PRGROMSize >262144) EMU->DbgOut(_T("PRG-ROM is larger than what original hardware supports!"));
	if (CART_BATTERY) eeprom =new EEPROM_24C01(0, ROM->PRGRAMData);
	LZ93D50::load(sync, eeprom, false);
	return TRUE;
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =LZ93D50::saveLoad(stateMode, offset, data);
	if (eeprom) offset =eeprom->saveLoad(stateMode, offset, data);	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =159;
} // namespace

MapperInfo MapperInfo_159 ={
	&mapperNum,
	_T("Bandai LZ93D50 with 24C01 EEPROM"),
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