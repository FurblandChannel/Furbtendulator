#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace  {
uint8_t		lastCHRBank;
FPPURead	readCHR;

void	sync (void) {
	MMC3::syncMirror();
	MMC3::syncPRG(0x3F, MMC3::getCHRBank(lastCHRBank) &0x02? 0x40: 0x00);
	MMC3::syncWRAM();
	EMU->SetCHR_RAM8(0x0, 0x0);
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	int result =readCHR(bank, addr);
	
	bank &=3;
	if (lastCHRBank !=bank) {
		lastCHRBank =bank;
		sync();
	}
	return result;
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	readCHR =EMU->GetPPUReadHandler(0x0);
	for (int bank =0; bank <8; bank++) {
		EMU->SetPPUReadHandler(bank, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, readCHR);
	}
}

uint16_t mapperNum =245;
} // namespace

MapperInfo MapperInfo_245 = {
	&mapperNum,
	_T("外星 FS003"),
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