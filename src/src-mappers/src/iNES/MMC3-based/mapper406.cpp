#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_FlashROM.h"

namespace {
FlashROM	*flash =NULL;

void	sync (void) {
	// no WRAM!
	MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	flash->write(bank, bank <<12 &0x1000 | MMC3::getPRGBank(bank >>1 &3) <<13 &0x6000 | addr, val);
	if (ROM->INES2_SubMapper ==1)
		MMC3::write(bank ^(bank <=0x9 || bank >=0xE? 6: 0), addr, val);
	else
		MMC3::write(bank, addr &2? 1: 0, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(ROM->INES2_SubMapper ==1? 0x01: 0xC2, 0xA4, ROM->ChipRAMData, ROM->ChipRAMSize, 65536); // Macronix MX29F040 (0)/AMD AM29F040 (1)
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) {
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
	MMC3::cpuCycle();
	if (flash) flash->cpuCycle();
}

uint16_t mapperNum =406;
} // namespace

MapperInfo MapperInfo_406 ={
	&mapperNum,
	_T("Impact Soft"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};