#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FlashROM.h"

namespace GTROM {
uint8_t		reg;
FlashROM	*flash =NULL;

void	sync (void) {
	ROM->dipValue =reg >>6;
	EMU->SetPRG_ROM32(0x8, reg &0xF);
	EMU->SetCHR_RAM8(0x0, reg >>4 &1   );
	EMU->SetCHR_RAM8(0x8, reg >>5 &1 |2);
}

int	MAPINT	writeOpenBus (int bank, int addr) {
	reg =*EMU->OpenBus;
	sync();
	return reg;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	flash->write(bank, bank <<12 &0x7000 | addr, val);
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_Version <2) EMU->SetCHRRAMSize(32768);

	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(0xBF, 0xB7, ROM->ChipRAMData, ROM->ChipRAMSize, 4096);

	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0xFF;
	sync();
	
	EMU->SetCPUReadHandler(0x5, writeOpenBus);
	EMU->SetCPUReadHandler(0x7, writeOpenBus);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	EMU->SetCPUWriteHandler(0x7, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) {
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

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =111;
MapperInfo MapperInfo_111 ={
	&mapperNum,
	_T("GTROM"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
} // namespace
