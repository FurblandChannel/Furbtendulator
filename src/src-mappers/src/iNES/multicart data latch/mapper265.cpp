#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::data &0x07 | Latch::addr >>2 &0x18 | Latch::addr >>3 &0xE0)
#define mirrorH !!(Latch::addr &0x0002)
#define nrom    !!(Latch::addr &0x0080)
#define lock    !!(Latch::addr &0x2000)

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom  );
	EMU->SetPRG_ROM16(0xC, prg |!nrom*7);
	EMU->SetCHR_RAM8(0, 0);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (lock) {
		Latch::data =val;
		sync();
	} else
		Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =265;
} // namespace

MapperInfo MapperInfo_265 = {
	&mapperNum,
	_T("T-262"),
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