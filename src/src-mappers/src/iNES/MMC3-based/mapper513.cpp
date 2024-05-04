#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		lastCHRBank;
FPPURead	readCHR;

void	sync (void) {	
	MMC3::syncPRG_2(0x3F, 0x3F, MMC3::getCHRBank(lastCHRBank) &0xC0, 0x00);
	MMC3::syncCHR_RAM(0x3F, 0);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	int result =readCHR(bank, addr);
	if (lastCHRBank !=bank) {
		lastCHRBank =bank;
		sync();
	}
	return result;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	MMC3::reset(ResetType);
	readCHR =EMU->GetPPUReadHandler(0x0);
	for (int bank =0; bank <8; bank++) {
		EMU->SetPPUReadHandler(bank, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, readCHR);
	}
}

uint16_t mapperNum =513;
} // namespace

MapperInfo MapperInfo_513 ={
	&mapperNum,
	_T("SA-9602B"),
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