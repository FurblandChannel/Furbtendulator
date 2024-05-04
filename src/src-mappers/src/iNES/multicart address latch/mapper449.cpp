#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define nrom    !!(Latch::addr &0x080)
#define dip     !!(Latch::addr &0x200 && ROM->dipValueSet)
#define prg       (Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20)

FCPURead	readCart;
int	MAPINT	readDIP (int bank, int addr);

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~(cpuA14*nrom));
	EMU->SetPRG_ROM16(0xC, prg |  cpuA14*nrom  |7*!nrom);
	
	iNES_SetCHR_Auto8(0x0, Latch::data);
	/* Must not write-protect CHR, as at least one menu uses NROM mode while writing to CHR RAM */
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
}

int	MAPINT	readDIP (int bank, int addr) {
	return readCart(bank, addr | ROM->dipValue);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);	
}

uint16_t mapperNum =449;
} // namespace


MapperInfo MapperInfo_449 = {
	&mapperNum,
	_T("22-in-1 King Series"),
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