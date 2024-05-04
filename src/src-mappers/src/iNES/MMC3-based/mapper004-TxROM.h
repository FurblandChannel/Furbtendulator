#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace TxROM {
void	sync (void) {
	MMC3::syncWRAM();

	MMC3::syncPRG(0xFF, 0x00); // Should be 0x3F if we were really strict.
	MMC3::syncCHR(0xFF, 0x00);
	
	if (ROM->INES2_SubMapper ==2) // TFROM with fixed mirroring.
		iNES_SetMirroring();
	else
	if (ROM->INES_Flags &8)
		EMU->Mirror_4();
	else
		MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	// Determine ASIC-specific PPU cycle handler
	switch (ROM->INES2_SubMapper) {
		case 3:  MMC3::load(sync, MMC3Type::Acclaim); break;
		case 4:  MMC3::load(sync, MMC3Type::NEC);     break;
		default: MMC3::load(sync, MMC3Type::Sharp);   break;
	}
	
	// Identify board name based on submapper and RAM/ROM sizes
	if (ROM->INES2_SubMapper ==3) {
		MapperInfo_004.Description = _T("Acclaim MC-ACC");
	} else if (ROM->INES_Flags &8) {
		if (ROM->INES_PRGSize <128/16)
			MapperInfo_004.Description = _T("Nintendo TVROM");
		else
			MapperInfo_004.Description = _T("Nintendo TR1ROM");
	} else if (ROM->INES_CHRSize ==0) {
		if (ROM->INES_Version >=2) {
			if (ROM->INES2_PRGRAM >0)
				MapperInfo_004.Description = _T("Nintendo TNROM");
			else
				MapperInfo_004.Description = _T("Nintendo TGROM");
		} else
			MapperInfo_004.Description = _T("Nintendo TGROM/TNROM");
	} else if (ROM->INES_PRGSize ==32/16) {
		MapperInfo_004.Description = _T("Nintendo TEROM");
	} else if (ROM->INES_PRGSize ==64/16) {
		MapperInfo_004.Description = _T("Nintendo TBROM");
	} else if (ROM->INES_CHRSize <128/8) {
		MapperInfo_004.Description = _T("Nintendo TFROM");
	} else if (ROM->INES_Version >=2) {
		if (ROM->INES2_PRGRAM >0) {
			if (ROM->INES_Flags &2)
				MapperInfo_004.Description = _T("Nintendo TKROM");
			else
				MapperInfo_004.Description = _T("Nintendo TKROM/TSROM");
		} else
			MapperInfo_004.Description = _T("Nintendo TLROM");
	} else if (ROM->INES_Flags &2)
		MapperInfo_004.Description = _T("Nintendo TKROM");
	else
		MapperInfo_004.Description = _T("Nintendo TKROM/TLROM/TSROM");
	
	if (ROM->PRGROMSize >524288) EMU->DbgOut(L"Warning: MMC3 with invalid >512 KiB of PRG-ROM!");
	return TRUE;
}

uint16_t mapperNum =4;
MapperInfo MapperInfo_TxROM ={
	&mapperNum,
	_T("Nintendo TxROM"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	::unload,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};
} // namespace
