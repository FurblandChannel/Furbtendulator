#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::data &0x07 | Latch::data >>1 &0x38)
#define lock    !!(Latch::data &0x08)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, prg |7);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (lock || Latch::data &0x80 || ~val &0x80) {
		Latch::data =Latch::data &~7 | val &7;
		sync();
	} else
		Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =324;
} // namespace

MapperInfo MapperInfo_324 ={
	&mapperNum,
	_T("FARID UNROM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};