#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>6 &0x7F)
#define chr       (Latch::data &0x03 | Latch::addr <<2 &0x3C)
#define nrom128 !!(Latch::addr &0x20)
#define mirrorH !!(Latch::addr &0x2000)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~!nrom128);
	EMU->SetPRG_ROM16(0xC, prg | !nrom128);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	// Map the 1.5 MiB ROM correctly into 2.0 MiB address space.
	// 128 KiB bank 3 follows bank 1 in the .NES file, so move it, and set bank 2 to open bus.
	if (ROM->PRGROMSize ==0x180000) {
		EMU->SetPRGROMSize(0x200000);
		for (int i =0x000000; i <0x080000; i++) {
			ROM->PRGROMData[0x180000 +i] =ROM->PRGROMData[0x100000 +i];
			ROM->PRGROMData[0x100000 +i] =i >>8 &0xFF;
		}
	}
	return TRUE;
}

uint16_t mapperNum =228;
} // namespace

MapperInfo MapperInfo_228 ={
	&mapperNum,
	_T("Action 52"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};