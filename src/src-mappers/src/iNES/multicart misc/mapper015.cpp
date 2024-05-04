#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	int prg =Latch::data &0x3F;
	switch (Latch::addr &3) {
		case 0:	EMU->SetPRG_ROM32(0x8, prg >>1);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, prg   );
			EMU->SetPRG_ROM16(0xC, prg |7);
			break;
		case 2:	EMU->SetPRG_ROM8(0x8, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xA, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xC, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xE, prg <<1 | Latch::data >>7);
			break;
		case 3:	EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
			break;
	}
	EMU->SetCHR_RAM8(0, 0);
	
	if (Latch::data &0x40)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD && ROM->INES_Flags &4) {
		// A few ROM hacks use 512-byte trainers; copy them into PRG-RAM.
		for (unsigned int i =0; i <ROM->MiscROMSize; i++) ROM->PRGRAMData[(0x1000 +i) &(ROM->PRGRAMSize -1)] =ROM->MiscROMData[i];
	}
	Latch::reset(RESET_HARD);
}

uint16_t MapperNum =15;
} // namespace

MapperInfo MapperInfo_015 ={
	&MapperNum,
	_T("K-1029"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};