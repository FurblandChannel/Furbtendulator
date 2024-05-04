#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetPRG_OB4(bank);
	int prg =Latch::data &0x3F | Latch::addr <<2 &0x40 | Latch::addr >>5 &0x80;
	switch (Latch::addr &7) {
		case 0: case 4:
			EMU->SetPRG_ROM32(0x8, prg >>1);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg |7);
			break;
		case 2:	case 6:
			EMU->SetPRG_ROM8(0x8, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xA, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xC, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM8(0xE, prg <<1 | Latch::data >>7);
			break;
		case 3:	case 7:
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
			break;
		case 5:	EMU->SetPRG_ROM8 (0x6, prg <<1 | Latch::data >>7);
			EMU->SetPRG_ROM32(0x8, prg >>1 | 3);
			break;
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &0x08) protectCHRRAM();
	
	if (Latch::data &0x40)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	MapperInfo_354.Description =ROM->INES_PRGSize <=512/16? _T("FAM250"): ROM->INES_PRGSize <=2048/16? _T("81-01-39-C"):  _T("SCHI-24");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank<=(ROM->INES2_SubMapper ==0? 0xE: 0xD); bank++) EMU->SetCPUWriteHandler(bank, EMU->WritePRG);
}

uint16_t MapperNum =354;
} // namespace

MapperInfo MapperInfo_354 ={
	&MapperNum,
	_T("FAM250/81-01-39-C/SCHI-24"),
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