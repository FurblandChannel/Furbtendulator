#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	MMC3::syncPRG(0x0F, reg &0xF0);
	MMC3::syncCHR_ROM(0x7F, reg <<3 &0x780);
	MMC3::syncWRAM();
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	reg =0;
	MMC3::reset(resetType);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	offset = MMC3::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =361;
} // namespace

MapperInfo MapperInfo_361 ={
	&mapperNum,
	_T("晶太 YY841101C"),
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