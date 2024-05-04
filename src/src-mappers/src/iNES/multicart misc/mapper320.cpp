#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define uorom   !(Latch::addr &0x10)
#define prg      (Latch::data &(uorom? 0x0F: 0x07) | Latch::addr <<3 &~7)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, prg | 7 |uorom*8);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (bank ==0xF && (addr &0xFE0) ==0x0E0)
		Latch::write(bank, addr, val);
	else {
		Latch::data =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =320;
} // namespace

MapperInfo MapperInfo_320 ={
	&mapperNum,
	_T("830425C-4391T"),
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