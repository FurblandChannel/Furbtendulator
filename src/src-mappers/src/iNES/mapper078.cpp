#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &0x7);
	EMU->SetPRG_ROM16(0xC, 0xF);
	EMU->SetCHR_ROM8(0, Latch::data >>4);
	if (ROM->INES2_SubMapper ==3) {
		if (Latch::data &0x8)
			EMU->Mirror_V();
		else
			EMU->Mirror_H();
	} else
		if (Latch::data &0x8)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
}

BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper ==1) MapperInfo_078.Description =L"Jaleco JF-16"; else
	if (ROM->INES2_SubMapper ==3) MapperInfo_078.Description =L"Irem IF-12";
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =78;
} // namespace

MapperInfo MapperInfo_078 ={
	&mapperNum,
	_T("Jaleco JF-16/Irem IF-12"),
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