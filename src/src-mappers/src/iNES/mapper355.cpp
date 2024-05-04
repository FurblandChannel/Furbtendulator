#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_PIC16C5x.h"

namespace {
uint16_t	cpuAddress;
FCPURead	readCPU [16];
FCPUWrite	writeCPU[16];
PIC16C54*	pic;


int	MAPINT	trapCPURead (int bank, int addr) {
	cpuAddress =bank <<12 |addr;
	return readCPU[bank](bank, addr);
}

void	MAPINT	trapCPUWrite (int bank, int addr, int val) {
	cpuAddress =bank <<12 |addr;
	writeCPU[bank](bank, addr, val);
}

int	picRead (int port) {
	if (port ==0) {
		return 1
		      |(cpuAddress &0x0040? 2: 0) // A6->RA1
		      |(cpuAddress &0x0020? 4: 0) // A5->RA2
		      |(cpuAddress &0x0010? 8: 0);// A4->RA3
	} else
	if (port ==1) {
		return (cpuAddress &0x1000?   1: 0) // A12->RA0
		      |(cpuAddress &0x0080?   2: 0) // A7 ->RA1
		      |(cpuAddress &0x0400?   4: 0) // A10->RA2
		      |(cpuAddress &0x0800?   8: 0) // A11->RA3
		      |(cpuAddress &0x0200?  16: 0) // A9 ->RA4
		      |(cpuAddress &0x0100?  32: 0) // A8 ->RA5
		      |(cpuAddress &0x2000?  64: 0) // A13->RA6
		      |(cpuAddress &0x4000? 128: 0);// A14->RA7
	} else
	return 0xFF;
}

void	picWrite (int port, int val) {
	if (port ==0) EMU->SetIRQ(val &0x1001? 1: 0);
}

BOOL	MAPINT	load (void) {
	if (pic) {
		delete pic;
		pic =NULL;
	}
	if (ROM->MiscROMSize ==1024) 
		pic =new PIC16C54(ROM->MiscROMData, picRead, picWrite);
	else
		MessageBox(NULL, _T("PIC16C54 ROM missing. Game will not run properly!"), _T("黃信維 3D-BLOCK"), MB_ICONERROR);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (int bank =0x0; bank<=0xF; bank++) {
		readCPU [bank] =EMU->GetCPUReadHandler(bank);
		writeCPU[bank] =EMU->GetCPUWriteHandler(bank);
		
		EMU->SetCPUReadHandler(bank, trapCPURead);
		EMU->SetCPUReadHandlerDebug(bank, readCPU[bank]);
		EMU->SetCPUWriteHandler(bank, trapCPUWrite);
	}
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
	if (pic) pic->reset(resetType);
}

void	MAPINT	unload (void) {
	if (pic) {
		delete pic;
		pic =NULL;
	}
}
void	MAPINT	cpuCycle (void) {
	if (pic) pic->run();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_WORD(stateMode, offset, data, cpuAddress);
	if (pic) offset =pic->saveLoad(stateMode, offset, data);
	return offset;
}

uint16_t mapperNum =355;
} // namespace

MapperInfo MapperInfo_355 ={
	&mapperNum,
	_T("黃信維 3D-BLOCK"),
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