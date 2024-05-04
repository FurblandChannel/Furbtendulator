#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::addr &0x4000)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	}
	EMU->SetCHR_ROM8(0, Latch::addr);
	if (Latch::addr &8)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();	
}

int	MAPINT	readProtection (int bank, int addr) {
	return *EMU->OpenBus | ((addr &0x10)? 0x00: 0x80);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	Latch::reset(resetType);
	EMU->SetCPUReadHandler(0x6, readProtection);
	EMU->SetCPUReadHandler(0x7, readProtection);
}

uint16_t mapperNum =212;
} // namespace

MapperInfo MapperInfo_212 ={
	&mapperNum,
	_T("CS669"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};
