#include	"h_KS202.h"

namespace KS202 {
uint8_t		index;
uint8_t		prg[4];
bool		rom6;
uint8_t		irqControl;
uint16_t	irqCounter;
uint16_t	irqLatch;
FSync		sync;

void	syncPRG (int AND, int OR) {
	if (rom6)
		EMU->SetPRG_ROM8(0x6, prg[3] &AND |OR);
	else
		EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM8(0x8, prg[0] &AND |OR);
	EMU->SetPRG_ROM8(0xA, prg[1] &AND |OR);
	EMU->SetPRG_ROM8(0xC, prg[2] &AND |OR);
	EMU->SetPRG_ROM8(0xE, 0xFF   &AND |OR);
}

void	syncPRG (int AND, int OR6, int OR8, int ORA, int ORC, int ORE) {
	if (rom6)
		EMU->SetPRG_ROM8(0x6, prg[3] &AND |OR6);
	else
		EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM8(0x8, prg[0] &AND |OR8);
	EMU->SetPRG_ROM8(0xA, prg[1] &AND |ORA);
	EMU->SetPRG_ROM8(0xC, prg[2] &AND |ORC);
	EMU->SetPRG_ROM8(0xE, 0xFF   &AND |ORE);
}

void	MAPINT	writeIRQcounter (int bank, int addr, int val) {
	val &=0xF;
	int shift =(bank &7) <<2;
	int mask =~(0xF <<shift);
	irqLatch =(irqLatch &mask) | (val <<shift);
}

void	MAPINT	writeIRQenable (int bank, int addr, int val) {
	irqControl =val;
	if (irqControl &2) irqCounter =irqLatch;
	EMU->SetIRQ(1);
}

void	MAPINT	writeIRQacknowledge (int bank, int addr, int val) {
	EMU->SetIRQ(1);
}

void	MAPINT	writePRGindex (int bank, int addr, int val) {
	index =val &0x7;
}

void	MAPINT	writePRGdata (int bank, int addr, int val) {
	if (index >=1 && index <=4) {
		prg[index -1] =val;
		sync();
	} else
	if (index ==5)
		rom6 =!!(val &4);
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	EMU->SetCPUWriteHandler(0x8, writeIRQcounter);
	EMU->SetCPUWriteHandler(0x9, writeIRQcounter);
	EMU->SetCPUWriteHandler(0xA, writeIRQcounter);
	EMU->SetCPUWriteHandler(0xB, writeIRQcounter);
	EMU->SetCPUWriteHandler(0xC, writeIRQenable);
	EMU->SetCPUWriteHandler(0xD, writeIRQacknowledge);
	EMU->SetCPUWriteHandler(0xE, writePRGindex);
	EMU->SetCPUWriteHandler(0xF, writePRGdata);

	if (resetType ==RESET_HARD) {
		for (int i =0; i <4; i++) prg[i] =i;
		index =0;
		irqControl =false;
		irqCounter =0;
		irqLatch =0;
		rom6 =false;
		EMU->SetIRQ(1);
	}
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (irqControl &2 && ++irqCounter ==0xFFFF) {
		irqCounter =irqLatch;
		irqControl &=~0x02;
		EMU->SetIRQ(0);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	SAVELOAD_WORD(stateMode, offset, data, irqLatch);
	SAVELOAD_BYTE(stateMode, offset, data, irqControl);
	SAVELOAD_BOOL(stateMode, offset, data, rom6);
	return offset;
}
}