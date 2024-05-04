#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::data &0x1F | Latch::addr <<3 &0x20)
#define mirrorH !!(Latch::data &0x20)
#define nrom    !!(Latch::data &0x80)
#define cpuA14  !!(Latch::data &0x40)
#define dip     !!(Latch::addr &0x01)
#define lock    !!(Latch::addr &0x02)

FCPURead	readCart;

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue;
}

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~(cpuA14 * nrom));
	EMU->SetPRG_ROM16(0xC, prg |  cpuA14 |!nrom*7);
	EMU->SetCHR_RAM8(0, 0);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (lock) {
		Latch::data =Latch::data &~7 | val &7;
		sync();
	} else
		Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t MapperNum =237;
} // namespace

MapperInfo MapperInfo_237 = {
	&MapperNum,
	_T("Teletubbies 420-in-1"),
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