#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if ((Latch::addr &0x9) ==0x9)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1);
	}
	EMU->SetCHR_ROM8(0, Latch::addr >>1);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	Latch::reset(resetType);
}

uint16_t mapperNum =202;
} // namespace

MapperInfo MapperInfo_202 ={
	&mapperNum,
	_T("SP60 150-in-1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};