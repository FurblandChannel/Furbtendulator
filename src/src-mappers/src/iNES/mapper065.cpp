#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prgInvert;
uint8_t		prg[2];
uint8_t		chr[8];
uint8_t		mirroring;
uint8_t		irq;
uint16_t	counter;
uint16_t	latch;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM8(0x8, prgInvert &0x80? 0xFE: prg[0]);
	EMU->SetPRG_ROM8(0xA, prg[1]);
	EMU->SetPRG_ROM8(0xC, prgInvert &0x80? prg[0]: 0xFE);
	EMU->SetPRG_ROM8(0xE, 0xFF  );
	
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	
	switch(mirroring >>6) {
		case 0: EMU->Mirror_V(); break;
		case 2: EMU->Mirror_H(); break;
		default:EMU->Mirror_S0(); break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[bank >>1 &1] =val;
	sync();
}

void	MAPINT	writeMisc (int bank, int addr, int val) {
	switch (addr &7) {
		case 0:	prgInvert =val;
			sync();
			break;
		case 1:	mirroring =val;
			sync();
			break;
		case 3: irq =val;
			EMU->SetIRQ(1);
			break;
		case 4:	counter =latch;
			EMU->SetIRQ(1);
			break;
		case 5:	latch =latch &0x00FF | val <<8;
			break;
		case 6:	latch =latch &0xFF00 | val;
			break;
	}
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &7] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType == RESET_HARD) {
		prgInvert =0;
		prg[0] =0x00;
		prg[1] =0x01;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		irq =0;
		counter =0;
		latch =0;
	}
	sync();

	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0xA, writePRG);
	
	EMU->SetCPUWriteHandler(0x9, writeMisc);
	EMU->SetCPUWriteHandler(0xB, writeCHR);
}


void	MAPINT	cpuCycle (void) {
	if (irq &0x80 && !--counter) {
		EMU->SetIRQ(0);
		irq =0;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prgInvert);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_WORD(stateMode, offset, data, latch);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =65;
} // namespace

MapperInfo MapperInfo_065 ={
	&mapperNum,
	_T("Irem H-3001"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};