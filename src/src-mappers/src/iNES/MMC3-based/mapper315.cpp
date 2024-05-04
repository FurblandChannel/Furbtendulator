#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	int outerBank =(reg >>1) &3;
	if (outerBank ==3)
		MMC3::syncPRG_GNROM_67(0x02, 0x0F, outerBank <<4);
	else
		MMC3::syncPRG(0x0F, outerBank <<4);
	MMC3::syncCHR_ROM(0xFF, ((reg &0x08)? 0x040: 0x000) | ((reg &0x02)? 0x080: 0x000) | ((reg &0x01)? 0x100: 0x000));
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
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

uint16_t mapperNum =315;
} // namespace

MapperInfo MapperInfo_315 = {
	&mapperNum,
	_T("830134C"),
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