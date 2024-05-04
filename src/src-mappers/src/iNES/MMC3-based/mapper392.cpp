#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	MMC3::syncWRAM();
	if (reg &0x10) {
		MMC3::syncPRG(0x0F, reg <<4);
		MMC3::syncCHR_ROM(0x7F, reg <<7);
	} else {
		EMU->SetPRG_ROM32(0x8, 0x20);
		EMU->SetCHR_RAM8(0x0, 0x00);
	}
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg &0x10) {
		reg =val;
		sync();
	}
	MMC3::wramWrite(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =392;
} // namespace

MapperInfo MapperInfo_392 = {
	&mapperNum,
	_T("00202650"),
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