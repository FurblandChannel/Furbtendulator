#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

// almost identical to 287
void	sync (void) {
	if (reg &8) {
		EMU->SetPRG_ROM32(0x8, reg &4 | reg >>4 &3);
	} else
		MMC3::syncPRG(0x0F, reg <<2 &~0x0F);
	MMC3::syncCHR(0x7F, reg <<5 &~0x7F);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	reg =val &0xFF;
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
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =321;
} // namespace

MapperInfo MapperInfo_321 = {
	&mapperNum,
	_T("820310"),
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