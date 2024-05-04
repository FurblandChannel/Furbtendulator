#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint8_t		irq;
uint16_t	counter;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, 0xF);
	EMU->SetPRG_ROM8(0x8, 0x8);
	EMU->SetPRG_ROM8(0xA, 0x9);
	EMU->SetPRG_ROM8(0xC, prg &8 | prg <<2 &4 | prg >>1 &3);
	EMU->SetPRG_ROM8(0xE, 0xB);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr &0x020 && ~addr &0x040) {
		if (addr &0x100)
			irq =val;
		else {
			prg =val;
			sync();
		}
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		irq =0;
		counter =0;
		EMU->SetIRQ(1);
	}
	sync();
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (irq &1)
		EMU->SetIRQ(++counter &0x1000? 0: 1);
	else {
		EMU->SetIRQ(1);
		counter =0;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =50;
} // namespace

MapperInfo MapperInfo_050 ={
	&mapperNum,
	_T("N-32 (761214)"),
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
