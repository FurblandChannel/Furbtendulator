#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	if (reg &0x08)
		MMC3::syncPRG_GNROM_66(0x02, 0x0F, reg <<4);
	else
		MMC3::syncPRG(0x0F, reg <<4);
	
	if (reg &0x04) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) | 0x300);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3) | 0x300);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) | 0x300);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7) | 0x300);
	} else
		MMC3::syncCHR(reg &0x02? 0xFF: 0x7F, reg <<7);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);	
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =765;
} // namespace

MapperInfo MapperInfo_765 ={
	&mapperNum,
	_T("820315-C"), /* (T-417) Street Fighter III+IV */
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