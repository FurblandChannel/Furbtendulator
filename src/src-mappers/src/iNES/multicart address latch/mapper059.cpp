#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define prg       (Latch::addr >>4)
#define chr       (Latch::addr &0x000F)
#define nrom256  !(Latch::addr &0x0080)
#define mirrorH !!(Latch::addr &0x0008)
#define dip     !!(Latch::addr &0x0100)
#define lock    !!(Latch::addr &0x0200)

FCPURead	readCart;

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue &0x03 | *EMU->OpenBus &~0x03;
}

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256);
	EMU->SetCHR_ROM8(0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (!lock) Latch::write(bank, addr, val);
}
BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =59;
} // namespace

MapperInfo MapperInfo_059 ={
	&mapperNum,
	_T("BS-01/VT1512A"),
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
