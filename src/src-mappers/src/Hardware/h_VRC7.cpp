#include	"h_VRC7.h"

namespace VRC7 {
FSync		sync;
uint8_t		A0, A1;
uint8_t		irq, counter, latch;
int16_t		cycles;
uint8_t		prg[3];
uint16_t	chr[8];
uint8_t		misc;

#define	irqRepeat  (irq &0x01)
#define	irqEnabled (irq &0x02)
#define	irqMode	   (irq &0x04)

void	MAPINT	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, prg[0] &AND |OR);
	EMU->SetPRG_ROM8(0xA, prg[1] &AND |OR);
	EMU->SetPRG_ROM8(0xC, prg[2] &AND |OR);
	EMU->SetPRG_ROM8(0xE,   0xFF &AND |OR);
	if (misc &0x80)
		EMU->SetPRG_RAM8(0x6, 0);
	else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
}

void	MAPINT	syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize)
		syncCHR_ROM(AND, OR);
	else
		syncCHR_RAM(AND, OR);
}

void	MAPINT	syncCHR_ROM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, (chr[bank] &AND) |OR);
}

void	MAPINT	syncCHR_RAM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_RAM1(bank, (chr[bank] &AND) |OR);
}

void	MAPINT	syncMirror() {
	switch (misc &3) {
		case 0:	EMU->Mirror_V(); break;
		case 1:	EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writePRGSound (int bank, int addr, int val) {
	int reg =(bank <<1 &2) | (addr &A0? 1: 0);
	if (reg <3) {
		prg[reg] =val;
		sync();
	} else
		VRC7sound::Write(bank <<12 |addr, val);
}

void	MAPINT  writeCHR (int bank, int addr, int val) {
	int reg =((bank -0xA) <<1) | (addr &A0? 1: 0);
		chr[reg] =val;
	sync();	
}


void	MAPINT	writeMisc (int bank, int addr, int val) {
	if (addr &A0)
		latch =val;
	else {
		misc =val;
		sync();
	}
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	if (addr &A0) {
		irq =irq &~2 | irq <<1 &2;
		EMU->SetIRQ(1);
	} else {
		irq =val;
		if (irqEnabled) {
			counter =latch;
			cycles =341;
		}
		EMU->SetIRQ(1);
	}
}

void	MAPINT	load (FSync _sync, uint8_t _A0, uint8_t _A1) {
	sync =_sync;
	A0 =_A0;
	A1 =_A1;
	VRC7sound::Load(true);
}


void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		prg[0] =0x00;
		prg[1] =0x01;
		prg[2] =0xFE;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		misc =0;
		irq =0;
		counter =0;
		latch =0;
		cycles =0;
	}
	EMU->SetIRQ(1);
	sync();

	VRC7sound::Reset(ResetType);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writePRGSound);
	for (int bank =0xA; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	EMU->SetCPUWriteHandler(0xE, writeMisc);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
}


void	MAPINT	unload (void) {
	VRC7sound::Unload();
}

void	MAPINT	cpuCycle (void) {
	if (irqEnabled && (irqMode || (cycles -= 3) <=0)) {
		if (!irqMode) cycles +=341;
		if (counter ==0xFF) {
			counter =latch;
			EMU->SetIRQ(0);
		} else
			counter++;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_WORD(stateMode, offset, data, cycles);
	SAVELOAD_BYTE(stateMode, offset, data, misc);
	for (int bank =0; bank <3; bank++) SAVELOAD_BYTE(stateMode, offset, data, prg[bank]);
	for (int bank =0; bank <8; bank++) SAVELOAD_BYTE(stateMode, offset, data, chr[bank]);
	offset = VRC7sound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int count) {
	return VRC7sound::Get(count) *(misc &0x40? 0: 4);
}

} // namespace VRC7;
