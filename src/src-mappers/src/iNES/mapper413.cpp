#include	"..\DLL\d_iNES.h"

namespace {
FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPUWrite	writeAPU;
uint8_t		reg[4];
uint32_t	serialAddr;
uint8_t		control;

uint8_t		counter;
uint8_t		reloadValue;
bool		enableIRQ;
uint8_t		pa12Filter;

void	sync (void) {
	EMU->SetPRG_ROM4(0x5, 0x01);
	EMU->SetPRG_ROM8(0x6, reg[0]);
	EMU->SetPRG_ROM8(0x8, reg[1]);
	EMU->SetPRG_ROM8(0xA, reg[2]);
	EMU->SetPRG_ROM4(0xD, 0x07);
	EMU->SetPRG_ROM8(0xE, 0x04);
	EMU->SetCHR_ROM4(0x0, reg[3]);
	EMU->SetCHR_ROM4(0x4, 0xFD);
	iNES_SetMirroring();
}

int	MAPINT	readPCM (int bank, int addr) {
	if (bank ==0x4 && ~addr &0x800)
		return readAPU(bank, addr);
	else
	if (control &2)
		return ROM->MiscROMData[serialAddr++ &(ROM->MiscROMSize -1)];
	else
		return ROM->MiscROMData[serialAddr   &(ROM->MiscROMSize -1)];
}
int	MAPINT	readPCMDebug (int bank, int addr) {
	if (bank ==0x4 && ~addr &0x800)
		return readAPUDebug(bank, addr);
	else
		return ROM->MiscROMData[serialAddr &(ROM->MiscROMSize -1)];
}

void	MAPINT	writeReloadValue (int bank, int addr, int val) {
	reloadValue =val;
}
void	MAPINT	writeReloadFlag (int bank, int addr, int val) {
	counter =0;
}
void	MAPINT	writeEnableIRQ (int bank, int addr, int val) {
	enableIRQ =!!(bank &1);
	if (!enableIRQ) EMU->SetIRQ(1);
}

void	MAPINT	writeSerialAddress (int bank, int addr, int val) {
	serialAddr = serialAddr <<1 | val >>7;
}
void	MAPINT	writeSerialControl (int bank, int addr, int val) {
	control =val;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[val >>6] =val &0x3F;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	serialAddr =0;
	control =0;

	counter =0;
	reloadValue =0;
	enableIRQ =false;
	pa12Filter =0;
	
	sync();
	
	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	EMU->SetCPUReadHandler     (0x4, readPCM);
	EMU->SetCPUReadHandlerDebug(0x4, readPCMDebug);
	EMU->SetCPUReadHandler     (0xC, readPCM);
	
	EMU->SetCPUWriteHandler(0x8, writeReloadValue);
	EMU->SetCPUWriteHandler(0x9, writeReloadFlag);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeEnableIRQ);
	EMU->SetCPUWriteHandler(0xC, writeSerialAddress);
	EMU->SetCPUWriteHandler(0xD, writeSerialControl);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			counter =!counter? reloadValue: --counter;
			if (!counter && enableIRQ) EMU->SetIRQ(0);
		}
		pa12Filter =5;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_LONG(stateMode, offset, data, serialAddr);
	SAVELOAD_BYTE(stateMode, offset, data, control);
	
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);

	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =413;
} // namespace

MapperInfo MapperInfo_413 = {
	&mapperNum,
	_T("BATMAP-SRR-X"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};