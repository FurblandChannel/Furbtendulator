#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	if (reg &8)
		MMC3::syncPRG_GNROM_67(2, 0x0F, reg <<4);
	else
		MMC3::syncPRG(0x0F, reg <<4);
	if (reg &4)
		MMC3::syncCHR_ROM(0x7F, reg <<6 &0x180);
	else
		MMC3::syncCHR_ROM(0xFF, reg <<6 &0x100);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =430;
} // namespace

MapperInfo MapperInfo_430 = {
	&mapperNum,
	_T("831031C/T-308"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};