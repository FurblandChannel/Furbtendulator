#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM8(0x0, 0);
	EMU->SetPRG_RAM8(0x6, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {	
	iNES_SetSRAM();	
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =241;
} // namespace

MapperInfo MapperInfo_241 ={
	&mapperNum,
	_T("BNROM with WRAM"),
	COMPAT_FULL,
	load,
	Latch::resetHard, // Too many games depend on bank 0 on reset
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
