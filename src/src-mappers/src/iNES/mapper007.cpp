#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x10)
		EMU->Mirror_S1();
	else	
		EMU->Mirror_S0();
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_PRGSize ==64/16)
		MapperInfo_007.Description =_T("Nintendo AN1ROM"); 
	else if (ROM->INES_PRGSize ==256/16)
		MapperInfo_007.Description =_T("Nintendo AOROM"); 
	else if (ROM->INES2_SubMapper ==1)
		MapperInfo_007.Description =_T("Nintendo ANROM/AOROM"); 
	else if (ROM->INES2_SubMapper ==2)
		MapperInfo_007.Description =_T("Nintendo AMROM/AOROM"); 
	else
		MapperInfo_007.Description =_T("Nintendo AMROM/ANROM/AOROM"); 
	Latch::load(sync, ROM->INES2_SubMapper ==2);
	return TRUE;
}
uint16_t mapperNum =7;
} // namespace

MapperInfo MapperInfo_007 ={
	&mapperNum,
	_T("Nintendo AxROM"),
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
