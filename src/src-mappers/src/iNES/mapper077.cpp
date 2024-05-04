#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &0x0F);
	EMU->SetCHR_ROM2(0, Latch::data >>4);
	EMU->SetCHR_RAM2(2, 0);
	EMU->SetCHR_RAM2(4, 1);
	EMU->SetCHR_RAM2(6, 2);
	
	EMU->SetCHR_RAM2(8, 3);
	EMU->SetCHR_NT1 (10, 0);
	EMU->SetCHR_NT1 (11, 1);
	
	EMU->SetCHR_RAM2(12, 3);	
	EMU->SetCHR_NT1 (14, 0);
	EMU->SetCHR_NT1 (15, 1);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =77;
} // namespace

MapperInfo MapperInfo_077 ={
	&mapperNum,
	_T("Irem LROG017"),
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