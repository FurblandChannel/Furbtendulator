#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
bool		locked;
	
void	sync (void) {
	if ((Latch::data &0x1F) ==2)
		EMU->SetPRG_ROM32(0x8, Latch::data >>1 &0x0F);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data &0x1F);
		EMU->SetPRG_ROM16(0xC, Latch::data &0x1F);
	}
	EMU->SetCHR_ROM8(0x0, Latch::data &0x1F);
	switch(Latch::data >>6) {
		case 0:	EMU->Mirror_Custom(0, 0, 0, 1); break;
		case 1:	EMU->Mirror_V(); break;
		case 2:	EMU->Mirror_H(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =740;
} // namespace

MapperInfo MapperInfo_740 ={
	&mapperNum,
	_T("N-80"), /* (N-82098) 30合卡 */
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
