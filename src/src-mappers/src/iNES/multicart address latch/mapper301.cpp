#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>2 &0x1F)
#define mirrorH !!(Latch::addr &0x002)
#define invert  !!(Latch::addr &0x080)
#define dip     !!(Latch::addr &0x100)
#define unrom   !!(Latch::addr &0x200)

FCPURead	readCart;
int	MAPINT	readDIP (int bank, int addr) {
	return readCart (bank, addr &0xFFE |ROM->dipValue);
}

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, prg &~(7*invert) |7*unrom);
	EMU->SetCHR_RAM8(0, 0);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);	
}

uint16_t mapperNum =301;
} // namespace

MapperInfo MapperInfo_301 ={
	&mapperNum,
	_T("8157"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};