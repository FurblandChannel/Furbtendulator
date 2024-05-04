#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_FlashROM.h"

namespace {
FlashROM	*flash =NULL;

void	sync (void) {
	MMC3::syncPRG(0x3F, 0);
	EMU->SetPRG_ROM8(0x8, 0x00);
	EMU->SetPRG_ROM8(0xE, 0x30);
	MMC3::syncCHR(0xFF, 0);
	MMC3::syncMirror();
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	switch (bank &~1) {
		case 0xA:
			MMC3::writeMirroringWRAM(0xA, 0, addr &1);
			break;
		case 0xC:
			MMC3::writeIRQConfig(0xC, 0, (addr &0xFF) -1);
			MMC3::writeIRQConfig(0xC, 1, 0);
			MMC3::writeIRQEnable(0xE, addr ==0xFF? 0: 1, 0);
			break;
		case 0xE:
			addr =addr <<2 &8 | addr &1;
			MMC3::writeReg(0x8, 0, 0x40); MMC3::writeReg(0x8, 1, addr <<3 |0);
			MMC3::writeReg(0x8, 0, 0x41); MMC3::writeReg(0x8, 1, addr <<3 |2);
			MMC3::writeReg(0x8, 0, 0x42); MMC3::writeReg(0x8, 1, addr <<3 |4);
			MMC3::writeReg(0x8, 0, 0x43); MMC3::writeReg(0x8, 1, addr <<3 |5);
			MMC3::writeReg(0x8, 0, 0x44); MMC3::writeReg(0x8, 1, addr <<3 |6);
			MMC3::writeReg(0x8, 0, 0x45); MMC3::writeReg(0x8, 1, addr <<3 |7);
			MMC3::writeReg(0x8, 0, 0x46); MMC3::writeReg(0x8, 1, 0x20 | addr);
			MMC3::writeReg(0x8, 0, 0x47); MMC3::writeReg(0x8, 1, 0x10 | addr);
			break;
	}
	flash->write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(0x37, 0x86, ROM->ChipRAMData, ROM->ChipRAMSize, 65536, 0x555, 0x2AA); // AMIC A29040B
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

uint16_t mapperNum =451;
} // namespace

MapperInfo MapperInfo_451 ={
	&mapperNum,
	_T("Impact Soft C-IM2-BASE"),
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