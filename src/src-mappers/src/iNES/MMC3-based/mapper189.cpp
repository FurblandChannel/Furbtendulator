#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, prg | prg >>4);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

void	MAPINT	interceptAPUWrite (int bank, int addr, int val) {
	if (bank ==4) EMU->WriteAPU(bank, addr, val);
	if (addr &0x100) {
		prg =val;
		sync();
	}
}

void	MAPINT	writeWRAM (int bank, int addr, int val) {
	prg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) prg =0;
	MMC3::reset(resetType);
	
	EMU->SetCPUWriteHandler(0x4, interceptAPUWrite);
	EMU->SetCPUWriteHandler(0x5, interceptAPUWrite);
	MMC3::setWRAMCallback(NULL, writeWRAM);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =189;
} // namespace

MapperInfo MapperInfo_189 ={
	&mapperNum,
	_T("TXC 01-22017-000/01-22018-400"),
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