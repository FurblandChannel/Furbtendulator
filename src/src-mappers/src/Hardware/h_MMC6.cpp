#include	"h_MMC6.h"

namespace MMC6 {
uint8_t		pointer;
uint8_t		reg[8];
uint8_t		mirroring;
uint8_t		wram;

uint16_t	counter;
uint8_t		reloadValue;
bool		enableIRQ;
uint8_t		pa12Filter;

FSync		sync;
FCPURead	wramRead;
FCPUWrite	wramWrite;

#define	prgInvert   (pointer &0x40)
#define	chrInvert   (pointer &0x80)
#define	wramEnabled (pointer &0x20)
#define write0      (wram &0x10)
#define read0       (wram &0x20)
#define write1      (wram &0x40)
#define read1       (wram &0x80)

void	syncWRAM (void) {
	if (!wramEnabled) wram =0x00;
	EMU->SetPRG_RAM4(0x7, 0);
}

int	getPRGBank (int bank) {
	if (~bank &1 && prgInvert) bank ^=2;
	return bank &2? 0xFE | bank &1: reg[6 | bank&1];
};

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, getPRGBank(0) &AND |OR);
	EMU->SetPRG_ROM8(0xA, getPRGBank(1) &AND |OR);
	EMU->SetPRG_ROM8(0xC, getPRGBank(2) &AND |OR);
	EMU->SetPRG_ROM8(0xE, getPRGBank(3) &AND |OR);
}

int	getCHRBank (int bank) {
	if (chrInvert) bank ^=4;
	if (bank &4)
		return reg[bank -2];
	else
		return reg[bank >>1] &~1 | bank&1;
};

void	syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize)
		syncCHR_ROM(AND, OR);
	else
		syncCHR_RAM(AND, OR);
}

void	syncCHR_ROM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR);
}

void	syncCHR_RAM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_RAM1(bank, getCHRBank(bank) &AND |OR);
}

void	syncMirror (void) {
	if (mirroring &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	trapWRAMRead (int bank, int addr) {
	// "If neither bank is enabled for reading, the $7000-$7FFF area is open bus.
	//  If only one bank is enabled for reading, the other reads back as zero."
	addr &=0x3FF;
	if (~addr &0x200)	// first 512 bytes
		return read0? wramRead(0x7, addr): (read1? 0x00: *EMU->OpenBus);
	else			// second 512 bytes
		return read1? wramRead(0x7, addr): (read0? 0x00: *EMU->OpenBus);
}

void	MAPINT	trapWRAMWrite (int bank, int addr, int val) {
	// "The write-enable bits only have effect if that bank is enabled for reading, otherwise the bank is not writable."
	addr &=0x3FF;
	if (~addr &0x200) {	// first 512 bytes
		if (read0 && write0) wramWrite(0x7, addr, val);
	} else {		// second 512 bytes 
		if (read1 && write1) wramWrite(0x7, addr, val);
	}
}

void	MAPINT	write (int bank, int addr, int val) {
	switch(bank &~1) {
		case 0x8: writeReg(bank, addr, val); break;
		case 0xA: writeMirroringWRAM(bank, addr, val); break;
		case 0xC: writeIRQConfig(bank, addr, val); break;
		case 0xE: writeIRQEnable(bank, addr, val); break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &1)
		reg[pointer &7] =val;
	else 
		pointer =val;
	sync();
}

void	MAPINT	writeMirroringWRAM (int bank, int addr, int val) {
	if (addr &1)
		wram =val;
	else
		mirroring =val;
	sync();
}

void	MAPINT	writeIRQConfig (int bank, int addr, int val) {
	if (addr &1)
		counter =0;
	else
		reloadValue =val;
}

void	MAPINT	writeIRQEnable (int bank, int addr, int val) {
	enableIRQ =!!(addr &1);
	if (!enableIRQ) EMU->SetIRQ(1);
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		pointer =0x00;
		reg[0] =0x00; reg[1] =0x02;
		reg[2] =0x04; reg[3] =0x05; reg[4] =0x06; reg[5] =0x07;
		reg[6] =0x00; reg[7] =0x01;
		mirroring =0;
		wram =0;
		enableIRQ =false;
		counter =0;
		pa12Filter =0;
	}
	wramRead  =EMU->GetCPUReadHandler (0x7);
	wramWrite =EMU->GetCPUWriteHandler(0x7);
	EMU->SetCPUReadHandler     (0x7, trapWRAMRead);
	EMU->SetCPUReadHandlerDebug(0x7, trapWRAMRead);
	EMU->SetCPUWriteHandler    (0x7, trapWRAMWrite);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeMirroringWRAM);
	for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeIRQConfig);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIRQEnable);
	EMU->SetIRQ(1);
	sync();
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
	SAVELOAD_BYTE(stateMode, offset, data, pointer);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, reg[i]);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, wram);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
} // namespace MMC6