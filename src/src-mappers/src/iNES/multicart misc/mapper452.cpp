#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync0 (void) {
	if (Latch::data &0x02) {
		EMU->SetPRG_ROM8(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM8(0xA, Latch::addr >>1);
		EMU->SetPRG_ROM8(0xC, Latch::addr >>1);
		EMU->SetPRG_ROM8(0xE, Latch::addr >>1);
		EMU->SetPRG_RAM8((Latch::data >>3 &6 |8) ^4, 0);
	} else
	if (Latch::data &0x08) {
		EMU->SetPRG_ROM8(0x8, Latch::addr >>1 &~1 |0);
		EMU->SetPRG_ROM8(0xA, Latch::addr >>1 &~1 |1);
		EMU->SetPRG_ROM8(0xC, Latch::addr >>1 &~1 |2);
		EMU->SetPRG_ROM8(0xE, Latch::addr >>1 &~1 |3 | Latch::data &0x04 | (Latch::data &0x04 && Latch::data &0x40? 0x8: 0x0));
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
		EMU->SetPRG_ROM16(0xC,               0);
	}
	EMU->SetPRG_RAM8(Latch::data >>3 &6 |8, 0);
	EMU->SetCHR_RAM8(0x0, 0);	
	if (Latch::data &0x01)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	sync1(void) {
	switch (Latch::addr &0xF000) {
	case 0xA000:
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM8 (0xC, 0);
		EMU->SetPRG_RAM8(Latch::addr >>8 &6 |0x8, 0);
		break;
	case 0xC000:
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1 |0);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1 |1);
		EMU->SetPRG_RAM8(Latch::addr >>8 &6 |0x8, 0);
		break;
	case 0xD000:
		EMU->SetPRG_ROM8(0x8, Latch::addr);
		EMU->SetPRG_ROM8(0xA, Latch::addr);
		EMU->SetPRG_ROM8(0xC, Latch::addr);
		EMU->SetPRG_ROM8(0xE, Latch::addr);
		EMU->SetPRG_RAM8(Latch::addr >>8 &2 |0x8, 0);
		EMU->SetPRG_RAM8(Latch::addr >>8 &2 |0xC, 0);
		break;
	case 0xE000:
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM16(0xC, Latch::addr &0x100? (Latch::addr >>1 |7): 0);
		EMU->SetPRG_RAM8(Latch::addr >>8 &6 |0x8, 0);
		break;
	default:
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM16(0xC, 0);
		break;
	}
	EMU->SetCHR_RAM8(0x0, 0);	
	if (Latch::addr &0x0800)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	EMU->WritePRG(bank, addr, val);
	if (ROM->INES2_SubMapper ==1 && bank <0xA || EMU->GetPRG_Ptr4(bank &~1) ==ROM->PRGRAMData)
		;
	else
		Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper ==1) {
		Latch::load(sync1, false);
		MapperInfo_452.Description =_T("DS-9-16");
	} else {
		Latch::load(sync0, false);
		MapperInfo_452.Description =_T("DS-9-27");
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =452;
} // namespace

MapperInfo MapperInfo_452 ={
	&mapperNum,
	_T("DS-9-27"),
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