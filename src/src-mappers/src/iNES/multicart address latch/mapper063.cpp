#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg          (Latch::addr >>2 &(ROM->INES2_SubMapper ==1? 0x7F: 0xFF))
#define	mirrorH    !!(Latch::addr &0x0001)
#define	nrom256    !!(Latch::addr &0x0002)
#define	protectCHR !!(Latch::addr &0x0400 && ROM->INES2_SubMapper==0 || Latch::addr &0x0200 && ROM->INES2_SubMapper==1)

namespace {
void	sync (void) {
	if (nrom256)
		EMU->SetPRG_ROM32(0x8, prg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	}
	if ((unsigned) prg <<14 >=ROM->PRGROMSize) for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	
	EMU->SetCHR_RAM8(0x0, 0);
	if (protectCHR) for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), FALSE);
	
	if (mirrorH)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =63;
} // namespace

MapperInfo MapperInfo_063 ={
	&mapperNum,
	_T("NTDEC 2291"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};