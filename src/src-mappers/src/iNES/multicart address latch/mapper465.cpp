#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>2 &0x1F | Latch::addr >>5 &0x20)
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define unrom   !!(Latch::addr &0x200)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8,  prg &~cpuA14  |Latch::data*unrom);
	EMU->SetPRG_ROM16(0xC, (prg | cpuA14) |          7*unrom);
	iNES_SetCHR_Auto8(0, 0);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (unrom)
		Latch::data =val;
	else
		Latch::addr =bank <<12 |addr;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =465;
} // namespace

MapperInfo MapperInfo_465 ={
	&mapperNum,
	_T("ET-120"),
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