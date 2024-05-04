#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		mirroring;
bool		irq;
uint16_t	counter;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM8(0x8, prg[0] |0x10);
	EMU->SetPRG_ROM8(0xA, prg[1]      );
	EMU->SetPRG_ROM8(0xC, prg[2]      );
	EMU->SetPRG_ROM8(0xE, prg[3] |0x10);
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	if (mirroring &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr &0xF) {
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			chr[addr &7] =val &(addr &4? 0xFF: 0xFE) | (addr &4? 0x00: addr &1? 0x01: 0x00);
			break;
		case 8: case 9: case 10: case 11:
			prg[addr &3] =val;
			break;
		case 12:
			mirroring =val &1;
			break;
		case 13:
			irq =false;
			counter =0;
			break;
		case 14:
			counter =counter &0xFF00 |val;
			break;
		case 15:
			counter =counter &0x00FF |val <<8;
			irq =true;
			break;
	}
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: prg) c =0;
		for (auto& c: chr) c =0;
		mirroring =0;
		irq =false;
		counter =0;
		EMU->SetIRQ(1);
	}
	sync();
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (counter !=0xFFFF) counter++;
	EMU->SetIRQ(counter ==0xFFFF && irq? 0: 1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BOOL(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =106;
} // namespace

MapperInfo MapperInfo_106 ={
	&mapperNum,
	_T("890418"),
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

