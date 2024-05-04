#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, Latch::data);
	EMU->SetPRG_ROM32(0x8, 0xFF);
	iNES_SetCHR_Auto8(0x0, 0);
	if (Latch::data &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t MapperNum =415;
} // namespace

MapperInfo MapperInfo_415 ={
	&MapperNum,
	_T("0353"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL,
};

