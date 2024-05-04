#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14   !(Latch::addr &0x001)
#define mirrorH   (ROM->INES2_SubMapper ==2? !!(Latch::addr &0x040): !!(Latch::addr &0x002))
#define protect !!(Latch::addr &0x080)
#define dip     !!(Latch::addr &0x100 && ROM->INES2_SubMapper ==0)
#define uorom   !!(Latch::addr &0x100 && ROM->INES2_SubMapper ==1)
#define nrom    !!(Latch::addr &0x200)
#define prg       (Latch::addr >>2 &0x1F)

FCPURead	readCart;
int	MAPINT	readDIP (int bank, int addr);

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~(cpuA14*nrom)                        );
	EMU->SetPRG_ROM16(0xC, prg |  cpuA14*nrom |7*!nrom | 8*!nrom*uorom);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (protect) protectCHRRAM();
	
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
	MapperInfo_380.Description =ROM->INES2_SubMapper ==2? L"9441109 4K1": ROM->INES2_SubMapper ==1? L"KN-35A": L"970630C";
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	readCart =EMU->GetCPUReadHandler(0x8);
	Latch::reset(RESET_HARD);	
}

uint16_t mapperNum =380;
} // namespace


MapperInfo MapperInfo_380 = {
	&mapperNum,
	_T("970630C/KN-35A"),
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