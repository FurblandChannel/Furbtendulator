#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\h_FlashROM.h"

namespace {
FlashROM	*flash =NULL;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_RAM8(0x0, Latch::data >>5);
	if (ROM->INES2_SubMapper ==1) {
		if (Latch::data &0x80)
			EMU->Mirror_V();
		else
			EMU->Mirror_H();
	} else
	switch(ROM->INES_Flags &9) {
		default:iNES_SetMirroring();
			break;
		case 8:	if (Latch::data &0x80)
				EMU->Mirror_S1();
			else
				EMU->Mirror_S0();
			break;
		case 9: EMU->SetCHR_RAM8(0x8, 3);
			break;
	}
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	if (bank ==0x8 && addr ==0x000) ROM->dipValue =val; // 8 BIT XMAS lights
	flash->write(bank, bank <<12 &0x3000 | Latch::data <<14 &0x4000 | addr, val);
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_Flags &2) {
		ROM->ChipRAMData =ROM->PRGROMData;
		ROM->ChipRAMSize =ROM->PRGROMSize;
		Latch::load(sync, false);
		flash =new FlashROM(0xBF, 0xB7, ROM->ChipRAMData, ROM->ChipRAMSize, 4096);
	} else
		Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	Latch::reset(resetType);
	if (ROM->INES_Flags &2) for (int bank =0x8; bank <=0xB; bank++) {
		EMU->SetCPUReadHandler(bank, readFlash);
		EMU->SetCPUReadHandlerDebug(bank, readFlash);
		EMU->SetCPUWriteHandler(bank, writeFlash);
	}
}

void	MAPINT	unload (void) {
	delete flash;
	flash =NULL;
}

void	MAPINT	cpuCycle() {
	if (flash) flash->cpuCycle();
}

uint16_t mapperNum =30;
} // namespace

MapperInfo MapperInfo_030 ={
	&mapperNum,
	_T("UNROM-512"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};