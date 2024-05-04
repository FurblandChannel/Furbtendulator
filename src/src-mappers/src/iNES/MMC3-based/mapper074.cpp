#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(0x3F, 0);
	if (ROM->INES_Version <2) {
		MMC3::syncCHR(0xFF, 0);
		for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), TRUE);
	} else
	for (int bank =0; bank <8; bank++) {
		int val =MMC3::getCHRBank(bank);
		if ((val &~1) ==0x8)
			EMU->SetCHR_RAM1(bank, val &1);
		else
			EMU->SetCHR_ROM1(bank, val);
	}
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

uint16_t mapperNum =74;
} // namespace

MapperInfo MapperInfo_074 ={
	&mapperNum,
	_T("43-393/860908C"),
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
