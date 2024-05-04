#include	"..\DLL\d_iNES.h"

namespace {
bool		mode;
bool		pa13;
uint16_t	ntAddr;
uint8_t		EXRAM[1024];

FPPURead	readPPU;
FPPUWrite	writePPU;

int	MAPINT	readCHR (int, int);
int	MAPINT	trapNTRead (int, int);

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM32(0x8, mode*0x01);
	iNES_SetMirroring();
}

int	MAPINT	readCHR (int bank, int addr) {
	pa13 =false;
	if (mode)
		return ROM->CHRROMData[EXRAM[ntAddr &0x3FF] <<11 &0x1F800 | bank <<9 | addr >>1 &~7 | addr &7];
	else
		return ROM->CHRROMData[                addr <<13 &0x10000 | bank <<9 | addr >>1 &~7 | addr &7];
}

int	MAPINT	trapNTRead (int bank, int addr) {
	if (!pa13) ntAddr =addr; // latch NT address on rising edges of PA13 (i.e. name table read, not attribute table read)
	pa13 =true;
	return readPPU(bank, addr);
}

void	MAPINT	writeEXRAM (int bank, int addr, int val) {
	EXRAM[addr &0x3FF] =val;
	writePPU(bank, addr, val);
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	mode =!mode;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readPPU =EMU->GetPPUReadHandler(0x0);
	writePPU =EMU->GetPPUWriteHandler(0x0);	
	mode =true;
	for (auto& c: EXRAM) c =0x00;
	
	sync();
	for (int bank =0x00; bank <0x08; bank++) {
		EMU->SetPPUReadHandler(bank, readCHR);
		EMU->SetPPUReadHandlerDebug(bank, readPPU);
	}
	for (int bank =0x08; bank <0x10; bank++) {
		EMU->SetPPUReadHandler(bank, trapNTRead);
		EMU->SetPPUReadHandlerDebug(bank, readPPU);
		EMU->SetCPUWriteHandler(bank, writeMode);
	}
	EMU->SetPPUWriteHandler(0x9, writeEXRAM);
	EMU->SetPPUWriteHandler(0xB, writeEXRAM);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, mode);
	for (auto& c: EXRAM) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =560;
} // namespace

MapperInfo MapperInfo_560 ={
	&mapperNum,
	_T("C/E BASIC"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};