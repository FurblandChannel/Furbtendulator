#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	//if (Latch::addr >=0xC010 && Latch::addr <=ROM->dipValue)
	if (~Latch::addr &0x100 && Latch::addr &ROM->dipValue)
		for (int bank =0xC; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (Latch::addr &0x2000)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1);
	}
	EMU->SetCHR_ROM8(0x0, Latch::data);
	if (Latch::addr &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

uint16_t MapperNum =414;
} // namespace

MapperInfo MapperInfo_414 ={
	&MapperNum,
	_T("9999999-in-1"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};