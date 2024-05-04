#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead	readCart;

int	MAPINT	readDIP (int bank, int addr) {
	return readCart(bank, addr | ROM->dipValue);
}

void	sync (void) {
	bool cpuA14  =!!(Latch::addr &0x001);
	bool mirrorH =!!(Latch::addr &0x002);
	bool nrom    =!!(Latch::addr &0x080);
	bool last    =!!(Latch::addr &0x200);
	bool dip     =!!(Latch::addr &0x400);
	bool variant =!!(ROM->INES2_SubMapper ==2);
	int  prg     =Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20; 

	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14);
	EMU->SetPRG_ROM16(0xC,(prg | cpuA14) &~(0xFF*variant*!nrom*!last) &~(7*!nrom*!last) |7*!nrom*last);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (!CART_BATTERY && nrom) protectCHRRAM();
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
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

uint16_t mapperNum =227;
} // namespace

MapperInfo MapperInfo_227 = {
	&mapperNum,
	_T("外星 FW01"),
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