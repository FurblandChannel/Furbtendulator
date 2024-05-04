#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		mirroring;
uint8_t		irqMode;

uint16_n	counter;
uint8_t		pa12Filter;
bool		reload;
bool		irqEnabled;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, prg[3]);
	EMU->SetPRG_ROM8(0x8, prg[0]);
	EMU->SetPRG_ROM8(0xA, prg[1]);
	EMU->SetPRG_ROM8(0xC, prg[2]);
	EMU->SetPRG_ROM8(0xE,   0xFF);

	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	
	switch (mirroring) {
		case 0:	EMU->Mirror_V();  break;
		case 1:	EMU->Mirror_H();  break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[addr &3] =val;
	sync();
}

void	MAPINT	write9 (int bank, int addr, int val) {
	// Both games just write 9000=FF, 9001=08, 9003=00 at reset
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	if (addr &8)
		;	// Both games just write $01 to $A008-$A00F all the time
	else
		chr[addr &7] =val;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	switch (addr &3) {
		case 0: counter.b0 =val;
			break;
		case 1:	counter.b1 =val;
			reload =true;
			break;
		case 2:	irqEnabled =false;
			break;
		case 3:	irqEnabled =true;
			break;
	}
	EMU->SetIRQ(1);
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val &3;
}

void	MAPINT	writeIRQMode (int bank, int addr, int val) {
	irqMode =val;
}

void	MAPINT	writeF (int bank, int addr, int val) {
	// Both games just write F000=00 at reset
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType == RESET_HARD) {
		for (int i =0; i <4; i++) prg[i] =0xFC |i;
		for (int i =0; i <8; i++) chr[i] =i;
		mirroring =0;
		irqMode =0;

		counter.s0 =0;
		pa12Filter =0;
		reload =false;
		irqEnabled =false;
	}
	sync();
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, write9);
	EMU->SetCPUWriteHandler(0xA, writeCHR);
	EMU->SetCPUWriteHandler(0xC, writeIRQ);
	EMU->SetCPUWriteHandler(0xD, writeMirroring);
	EMU->SetCPUWriteHandler(0xE, writeIRQMode);
	EMU->SetCPUWriteHandler(0xF, writeF);
}

void	MAPINT	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
	if (irqEnabled && ~irqMode &0x02 && counter.s0 && !--counter.s0) EMU->SetIRQ(0);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (irqMode &0x01 && addr &0x1000) {
		if (!pa12Filter && irqMode &0x02) {
			if (!counter.b0 || reload)
				counter.b0 =counter.b1;
			else
				counter.b0--;
			if (!counter.b0 && irqEnabled) EMU->SetIRQ(0);
			reload =false;
		}
		pa12Filter =5;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(stateMode, offset, data, prg[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, chr[i]);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, irqMode);
	SAVELOAD_WORD(stateMode, offset, data, counter.s0);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BOOL(stateMode, offset, data, reload);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum117 =117;
} // namespace

MapperInfo MapperInfo_117 = {
	&MapperNum117,
	_T("Future Media"),
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

