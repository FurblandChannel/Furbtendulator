#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	Sync (void) {
	MMC3::syncWRAM();
	for (int bank =0; bank <4; bank++) {
		int val =MMC3::getPRGBank(bank);
		EMU->SetPRG_ROM8(0x8 +bank*2, (val &0xF3) | ((val &0x08) >>1) | ((val &0x04) <<1));
	}
	for (int bank =0; bank <8; bank++) {
		int val =MMC3::getCHRBank(bank);
		EMU->SetCHR_ROM1(bank, (val &0xDD) | ((val &0x20) >>4) | ((val &0x02) <<4));
	}
	MMC3::syncMirror();
}

void	MAPINT	writeASIC (int Bank, int Addr, int Val) {
	MMC3::write(Bank, Addr >>3, Val);
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	MMC3::reset(ResetType);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeASIC);
}

uint16_t MapperNum =325;
} // namespace

MapperInfo MapperInfo_325 = {
	&MapperNum,
	_T("Mali Splash Bomb"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};
