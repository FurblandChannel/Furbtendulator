#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::data &0x20) {
		EMU->SetPRG_ROM16(0x8, Latch::data &0x1F);
		EMU->SetPRG_ROM16(0xC, Latch::data &0x1F);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::data >>1 &0x0F);
	EMU->SetCHR_RAM8(0x0, 0x0);
	if (~Latch::data &0x80) for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), FALSE);
	
	if (Latch::data &0x40)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}
	
BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

uint16_t MapperNum =433;
} // namespace

MapperInfo MapperInfo_433 ={
	&MapperNum,
	_T("Realtec NC-20MB"),
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

