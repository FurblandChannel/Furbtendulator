#include	"..\DLL\d_iNES.h"

namespace {
FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPURead	readCart;

int	MAPINT	read4 (int bank, int addr) {
	if (addr &0x800)
		return readCart(bank, addr);
	else
		return readAPU(bank, addr);
}
int	MAPINT	read4Debug (int bank, int addr) {
	if (addr &0x800)
		return readCart(bank, addr);
	else
		return readAPUDebug(bank, addr);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->INES_PRGSize ==16/16)
		MapperInfo_000.Description = _T("Nintendo NROM-128");
	else
	if (ROM->INES_PRGSize ==48/16)
		MapperInfo_000.Description = _T("Nintendo NROM-368");
	else
		MapperInfo_000.Description = _T("Nintendo NROM-256");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD && ROM->INES_Flags &4) {
		// A few ROM hacks use 512-byte trainers; copy them into PRG-RAM.
		for (unsigned int i =0; i <ROM->MiscROMSize; i++) ROM->PRGRAMData[(0x1000 +i) &(ROM->PRGRAMSize -1)] =ROM->MiscROMData[i];
	}
	iNES_SetMirroring();
	EMU->SetPRG_RAM8(0x6, 0); // For Family BASIC and trainered ROM hacks. Will be open bus if no RAM specified in NES 2.0 header
	if (ROM->INES_PRGSize ==48/16) {
		EMU->SetPRG_ROM16(0x4, 0);
		EMU->SetPRG_ROM16(0x8, 1);
		EMU->SetPRG_ROM16(0xC, 2);
		readAPU      =EMU->GetCPUReadHandler     (0x4);
		readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
		readCart     =EMU->GetCPUReadHandler     (0x8);
		EMU->SetCPUReadHandler     (0x4, read4);
		EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	} else
		EMU->SetPRG_ROM32(0x8, 0);
	iNES_SetCHR_Auto8(0x0, 0);
}

uint16_t mapperNum =0;
} // namespace

MapperInfo MapperInfo_000 ={
	&mapperNum,
	_T("Nintendo NROM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};