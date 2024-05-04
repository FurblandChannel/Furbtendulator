#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg                 (Latch::addr &0x1F | Latch::addr >>3 &0x60)
#define	nrom128             (Latch::addr &0x0800)
#define oneScreenMirroring  (Latch::addr &0x0400)
#define a14                 (Latch::addr >>12 &1)
#define horizontalMirroring (Latch::addr &0x2000)

namespace {
bool	unrom;

void	sync (void) {
	if (unrom) {
		EMU->SetPRG_ROM16(0x8, ROM->INES_PRGSize &0xC0 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, ROM->INES_PRGSize &0xC0 | 7);
		EMU->SetCHR_RAM8(0x0, 0);
		EMU->Mirror_V();
		
		*EMU->multiCanSave  =FALSE;
	} else {
		if ((unsigned) prg >=ROM->PRGROMSize /32768)
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
		else
		if (nrom128) {
			EMU->SetPRG_ROM16(0x8, prg <<1 |a14);
			EMU->SetPRG_ROM16(0xC, prg <<1 |a14);
		} else
			EMU->SetPRG_ROM32(0x8, prg);		
		
		EMU->SetCHR_RAM8(0x0, 0);
		
		if (oneScreenMirroring)
			EMU->Mirror_S0();
		else
		if (horizontalMirroring) { // Horizontal mirroring also protects CHR-RAM as a side effect
			for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), FALSE);
			EMU->Mirror_H();
		} else
			EMU->Mirror_V();
		
		*EMU->multiCanSave   =TRUE;
		*EMU->multiPRGStart  =ROM->PRGROMData +(((prg *32768) +(nrom128 && a14? 16384: 0)) & (ROM->PRGROMSize -1));
		*EMU->multiPRGSize   =nrom128? 16384: 32768;
		*EMU->multiCHRStart  =ROM->CHRRAMData;
		*EMU->multiCHRSize   =8192;
		*EMU->multiMapper    =0;
		*EMU->multiMirroring =horizontalMirroring? 1: 0;
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	// If there is a second chip with an UNROM game, then soft-resetting switches between normal multi and UNROM game mode
	if (ROM->PRGROMSize &0x20000) unrom =resetType ==RESET_HARD? 1: !unrom;
	Latch::reset(RESET_HARD);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_AD(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, unrom);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =235;
} // namespace

MapperInfo MapperInfo_235 ={
	&mapperNum,
	_T("Golden Game modular multicart"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};