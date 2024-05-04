#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint8_t		irq;
uint16_t	counter;

const uint8_t	prg2bank[8] ={4,3,4,4,4,7,5,6};

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, 2);
	EMU->SetPRG_ROM8(0x8, 1);
	EMU->SetPRG_ROM8(0xA, 0);
	EMU->SetPRG_ROM8(0xC, prg2bank[prg &7]);
	EMU->SetPRG_ROM8(0xE, 9);
	iNES_SetCHR_Auto8(0, 0);
	iNES_SetMirroring();
}

int	MAPINT	readPRG2 (int bank, int addr) {
	return ROM->PRGROMData[0x10000 | ROM->dipValue | addr];
}

void	MAPINT	writePRG_IRQ (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr ==0x022) {
		prg =val;
		sync();
	} else
	if (addr ==0x122) {
		irq =val;
		if (val &2) counter =0;
	}
}

void	MAPINT	writeIRQhacked (int bank, int addr, int val) {
	if (addr ==0x122) {
		irq =val;
		if (val &2) counter =0;
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		irq =0;
		counter =0;
	}
	sync();
	EMU->SetCPUWriteHandler(0x4, writePRG_IRQ);
	EMU->SetCPUWriteHandler(0x8, writeIRQhacked);

	// The unhacked version uses 2 KiB at $5000-$57FF. In the ROM file, this is repeated three times to fill up 8 KiB bank #8,
	// to be followed by the modified FDS BIOS in 8 KiB bank #9, creating ten 8 KiB banks.
	// A hacked version replaces the 2 KiB ROM chip with an 8 KiB ROM chip, allowing A11 and A12 to select one of four title screen variants.
	if (!ROM->dipValueSet) ROM->dipValue =0x800;
	EMU->SetCPUReadHandler(0x5, readPRG2);
	EMU->SetCPUReadHandlerDebug(0x5, readPRG2);
}

void	MAPINT	cpuCycle (void) {
	EMU->SetIRQ(++counter &0x1000 && irq &1? 0: 1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =43;
} // namespace

MapperInfo MapperInfo_043 ={
	&mapperNum,
	_T("TONY-I"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};