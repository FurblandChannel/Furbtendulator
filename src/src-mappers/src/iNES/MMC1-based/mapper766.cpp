#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		outerBank;
	
void	sync (void) {
	MMC1::syncPRG(0x07, outerBank <<3);
	MMC1::syncCHR(0x1F, outerBank <<5);
	MMC1::syncWRAM(0);
	MMC1::syncMirror();
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	if (~outerBank &8) outerBank =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1B);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	MMC1::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x6, writeExtra);
	EMU->SetCPUWriteHandler(0x7, writeExtra);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =766;
} // namespace

MapperInfo MapperInfo_766 ={
	&mapperNum,
	_T("FC-41"), /* (F-647) High K All-New Sport 4-in-1 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
