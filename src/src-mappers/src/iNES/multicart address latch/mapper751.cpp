#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define protCHR !!(Latch::addr &0x080)
#define unrom   !!(Latch::addr &0x200)
#define prg       (Latch::addr >>2 &0x1F) 

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
	EMU->SetPRG_ROM16(0xC,(prg | cpuA14) |7*unrom);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (protCHR) protectCHRRAM();
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (unrom)
		Latch::addr =Latch::addr &~0x1C | addr &0x1C;
	else
		Latch::write(bank, addr, val);
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =751;
} // namespace

MapperInfo MapperInfo_751 = {
	&mapperNum,
	_T("T-215"), /* 超級金卡 9-in-1 128K */
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