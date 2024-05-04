#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0); // For Alwa's Awakening
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM(); // For Alwa's Awakening
	MapperInfo_002.Description =ROM->INES_PRGSize <=128/16? _T("Nintendo UNROM"): _T("Nintendo UOROM");
	Latch::load(sync, ROM->INES2_SubMapper ==2);
	return TRUE;
}

uint16_t mapperNum =2;
} // namespace

MapperInfo MapperInfo_002 ={
	&mapperNum,
	_T("Nintendo UxROM"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};