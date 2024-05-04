#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM8(0x6, ROM->PRGROMSize &0x6000? 0x20: 0x1F);
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {	
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =283;
} // namespace

MapperInfo MapperInfo_283 ={
	&mapperNum,
	_T("GS-2004/GS-2013"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
