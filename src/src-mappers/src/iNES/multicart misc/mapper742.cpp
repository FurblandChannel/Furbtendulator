#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
bool		locked;
	
void	sync (void) {
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &0x60) {
		EMU->SetPRG_ROM32(0x8, Latch::data &(Latch::addr &0x40? 7: 3) | Latch::addr >>2 &~7);
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else	
			EMU->Mirror_S0();
	} else {
		EMU->SetPRG_ROM32(0x8, Latch::addr &3 | Latch::addr >>2 &~3);
		EMU->Mirror_V();
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	Latch::data =val;
	if (locked) {
		int mask =Latch::addr &0x40? 7: 3;
		Latch::addr =Latch::addr &~mask | addr &mask;
	} else {
		Latch::addr =addr;
		locked =true;
	}
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	locked =false;
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =742;
} // namespace

MapperInfo MapperInfo_742 ={
	&mapperNum,
	_T("FC-40"), /* (F-685) High K Battletoads 11-in-1 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};
