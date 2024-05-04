#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();	
}

void	MAPINT	writeMMC3 (int bank, int addr, int val) {
	MMC3::write(bank, addr >>10, addr &0xFF);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeMMC3);
}

uint16_t mapperNum =250;
} // namespace

MapperInfo MapperInfo_250 ={
	&mapperNum,
	_T("L4015"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};