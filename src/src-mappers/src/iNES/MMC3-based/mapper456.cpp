#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg;

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(0xF, reg <<4);
	MMC3::syncCHR_ROM(0x7F, reg <<7);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	interceptAPUWrite (int bank, int addr, int val) {
	if (bank ==4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, interceptAPUWrite);
	EMU->SetCPUWriteHandler(0x5, interceptAPUWrite);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =456;
} // namespace

MapperInfo MapperInfo_456 ={
	&mapperNum,
	_T("Realtec K6C3001A"),
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