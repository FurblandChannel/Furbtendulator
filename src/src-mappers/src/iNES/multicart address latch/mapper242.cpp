#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define nrom    !!(Latch::addr &0x080)
#define last    !!(Latch::addr &0x200)
#define dip     !!(Latch::addr &0x100 && ROM->dipValueSet)
#define prg       (Latch::addr >>2 &0x1F)

FCPURead	readCart;
int	MAPINT	readDIP (int bank, int addr);

void	sync (void) {
	int prgBank =prg;

	bool haveTwoChips =ROM->PRGROMSize &0x20000 && ROM->PRGROMSize >0x20000; // There are two PRG-ROM chip if there are 128 KiB of extra PRG data following a power-of-two PRG data
	if (haveTwoChips) {
		if (Latch::addr &0x600) // Original chip
			prgBank &=(ROM->INES_PRGSize &~8) -1;        // Data is masked to power-of-two PRG data
		else                    // Extra chip
			prgBank  =(prg &7) +(ROM->INES_PRGSize &~8); // Data is masked to 128 KiB (8x16 KiB) and starts after the power-of-two PRG data
	}
	
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prgBank &~cpuA14);
	EMU->SetPRG_ROM16(0xC,(prgBank | cpuA14) &~(7*!nrom*!last) |7*!nrom*last);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (!CART_BATTERY && nrom && ROM->PRGROMSize >=512*1024) protectCHRRAM();
	
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
	MapperInfo_242.Description =ROM->PRGROMSize ==640*1024? L"ET-113": L"43272";
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);	
}

uint16_t mapperNum =242;
} // namespace

MapperInfo MapperInfo_242 = {
	&mapperNum,
	_T("43272"),
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