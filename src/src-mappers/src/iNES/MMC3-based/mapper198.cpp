#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM4(0x5, 2);
	MMC3::syncWRAM();
	for (int bank =0; bank <4; bank++) {
		int val =MMC3::getPRGBank(bank);
		if (val >=0x40) val =val &0x0F |0x40;
		EMU->SetPRG_ROM8(0x8 +bank*2, val);
	}
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

uint16_t mapperNum =198;
} // namespace

MapperInfo MapperInfo_198 ={
	&mapperNum,
	_T("TNROM-640"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};