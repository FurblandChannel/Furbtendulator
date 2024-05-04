#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM8(0x8 ^VRC24::prgFlip, VRC24::prg[0]);
	EMU->SetPRG_ROM8(0xA                , VRC24::prg[1]);
	EMU->SetPRG_ROM8(0xC ^VRC24::prgFlip, ROM->INES_PRGSize*2 -2);
	EMU->SetPRG_ROM8(0xE                , ROM->INES_PRGSize*2 -1);	
	VRC24::syncCHR(0x1FF, 0x000);
	VRC24::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x05, 0x0A, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	VRC24::reset(resetType);
	if (ROM->MiscROMData && ROM->MiscROMSize) for (unsigned int i =0; i <ROM->MiscROMSize; i++) ROM->PRGRAMData[i +0x1000] =ROM->MiscROMData[i];
}

uint16_t mapperNum =563;
} // namespace

MapperInfo MapperInfo_563 = {
	&mapperNum,
	_T("Bad mapper 23 homebrew"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};