#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t	prg, irqEnabled;
uint16_t irqCounter;
FCPURead readCart;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, prg);
	EMU->SetPRG_ROM32(0x8, 3);
	EMU->SetCHR_RAM8(0x0, 0);
}

int	MAPINT	readRAM (int bank, int addr) {
	int fullAddr =(bank <<12) |addr;
	if (fullAddr >=0xB800 && fullAddr <0xD800)
		return ROM->PRGRAMData[fullAddr -0xB800];
	else
		return readCart(bank, addr);
}

void	MAPINT	writeRAM (int bank, int addr, int val) {
	int fullAddr =(bank <<12) |addr;
	if (fullAddr >=0xB800 && fullAddr <0xD800)
		ROM->PRGRAMData[fullAddr -0xB800] =val;
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	irqEnabled =val &2;
	irqCounter =0;
	EMU->SetIRQ(1);
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	if (resetType ==RESET_HARD) prg =irqEnabled =irqCounter =0;

	readCart =EMU->GetCPUReadHandler(0x8);
	for (int i =0xB; i<=0xD; i++) {
		EMU->SetCPUReadHandler(i, readRAM);
		EMU->SetCPUReadHandlerDebug(i, readRAM);
		EMU->SetCPUWriteHandler(i, writeRAM);
	}
	EMU->SetCPUWriteHandler(0xE, writeIRQ);
	EMU->SetCPUWriteHandler(0xF, writePRG);
	FDSsound::ResetBootleg(resetType);
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (irqEnabled && ++irqCounter ==7560) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, irqEnabled);
	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =535;
} // namespace

MapperInfo MapperInfo_535 ={
	&MapperNum,
	_T("Whirlwind Manu LH53"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};