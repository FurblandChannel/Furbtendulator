#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	MMC3::syncPRG(reg <<1 |0x07, (reg | reg &2 & reg <<1) <<2 &~0x07);
	MMC3::syncCHR_ROM(0x7F, reg <<5 &~0x7F);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

BOOL	MAPINT	load (void)  {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =37;
} // namespace

MapperInfo MapperInfo_037 ={
	&mapperNum,
	_T("Nintendo ZZ"),
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