#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

/*    D~[..LH PPPP]
	   || |  +- 1=NROM-128, 0=NROM-256 if D3=1
	   || ++++- PRG A17..A14
	   || +---- 1=NROM, 0=UNROM
	   |+------ 1=H mirroring, 0=V mirroring
	   +------- 1=Lock */

#define lock    !!( Latch::data &0x20)
#define mirrorH !!( Latch::data &0x10)
#define nrom    !!( Latch::data &0x08)
#define cpuA14  !!(~Latch::data &0x01 && nrom)
#define prg       ( Latch::data &0x0F)

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
	EMU->SetPRG_ROM16(0xC, prg | cpuA14  |7*!nrom);
	iNES_SetCHR_Auto8(0x0, 0);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (!lock) Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	Latch::data =0x08;
	sync();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =277;
} // namespace


MapperInfo MapperInfo_277 = {
	&mapperNum,
	_T("09-078"),
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