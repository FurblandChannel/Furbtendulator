#include	"..\DLL\d_iNES.h"

namespace {
#define WRAM6  0
#define WRAM7  1
#define	PRG8   2
#define	PRGA   3
#define	PRGC   4
#define	CHR    5
#define CNTLO  6
#define CNTHI  7
#define IRQCTL 9
#define NTCTL  10
#define JISFLG 11
#define JISCOL 12
#define JISROW 13

#define	writeToEXRAM        (reg[NTCTL] &1)
#define	horizontalMirroring (reg[NTCTL] &2)
#define irqAcknowledge      (reg[IRQCTL] &1)
#define irqEnabled          (reg[IRQCTL] &2)

uint8_t		reg[16];
uint8_t		ntram[2048];

uint16_t	irqCounter;

FCPURead	readCart;
FPPURead	readPPU;
FPPUWrite	writePPU;

uint8_t		qtByte;
uint16_t	lastNTAddr;
bool		isSprite;

void	sync (void) {
	EMU->SetPRG_RAM4(0x6, (reg[WRAM6] &1) | (reg[WRAM6] >>2));
	EMU->SetPRG_RAM4(0x7, (reg[WRAM7] &1) | (reg[WRAM7] >>2));
	EMU->SetPRG_ROM8(0x8, reg[PRG8] &0x40? ((reg[PRG8] &0x3F) +0x10): (reg[PRG8] &0x0F));
	EMU->SetPRG_ROM8(0xA, reg[PRGA] &0x40? ((reg[PRGA] &0x3F) +0x10): (reg[PRGA] &0x0F));
	EMU->SetPRG_ROM8(0xC, reg[PRGC] &0x40? ((reg[PRGC] &0x3F) +0x10): (reg[PRGC] &0x0F));
	EMU->SetPRG_ROM8(0xE, 0x4F);
	
	EMU->SetCHR_RAM4(0x0, reg[CHR] &1);
	EMU->SetCHR_RAM4(0x4, 1);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

static const uint8_t pageTable[0x24] ={
	0x0,0x0,0x2,0x2,0x1,0x1,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,0xD,0xD
};

int	MAPINT	readReg (int bank, int addr) {
	if (addr ==0xC00 || addr ==0xD00) {
		uint8_t row =reg[JISROW] -0x20;
		uint8_t col =reg[JISCOL] -0x20;
		if (row <0x60 && col <0x60) {
			uint16_t code = (col %32)		// First, go through 32 columns of a column-third.
			               +(row %16)*32 		// Then, through 16 rows of a row-third.
			               +(col /32)*32*16		// Then, through three column-thirds.
			               +(row /16)*32*16*3 	// Finally, through three row-thirds.
			;
			uint16_t glyph =(code &0xFF) | (pageTable[code >>8] <<8);
			uint32_t tile =glyph *4; // four tiles per glyph
			if (addr ==0xC00) // tile number
				return (tile &0xFF) | (reg[JISFLG] &3); 
			else	// bank byte
				return (tile >>8) | (reg[JISFLG] &0x4? 0x80: 0x00) | 0x40;
		} else
			return 0;
	} else
		return readCart(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr >>8] =val;
	switch (addr >>8) {
		case 0x8:
			if (irqAcknowledge)
				reg[IRQCTL] |=2;
			else
				reg[IRQCTL] &=~2;
			EMU->SetIRQ(1);
			break;
		case 0x9:
			reg[IRQCTL] =val;
			if (irqEnabled) irqCounter =reg[CNTLO] | (reg[CNTHI] <<8);
			EMU->SetIRQ(1);
			break;
	}
	sync();
}

int	getNTBank(int bank) {
	return ((horizontalMirroring? (bank >>1): bank) &1) *1024;
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	if (qtByte &0x40) {
		if (addr &0x08)
			return qtByte &0x80? 0xFF: 0x00;
		else {
			int fullAddr =((qtByte &0x3F) <<12) | ((bank &3) <<10) |addr;
			if (ROM->CHRROMSize ==128*1024) fullAddr =((fullAddr &0x00007) <<1) | ((fullAddr &0x00010) >>4) | ((fullAddr &0x3FFE0) >>1);
			return ROM->CHRROMData[fullAddr];
		}
	} else
	if (!isSprite)
		return ROM->CHRRAMData[((qtByte &0x01) <<12) | (((bank <<10) | addr) &0xFFF)];
	else
		return readPPU(bank, addr);
}

int	MAPINT	interceptNTRead (int bank, int addr) {
	if (addr <0x3C0) qtByte =ntram[getNTBank(bank) +addr];
	
	isSprite =lastNTAddr ==addr;
	lastNTAddr =addr;
	
	return readPPU(bank, addr);
}

void	MAPINT	interceptNTWrite (int bank, int addr, int val) {
	if (writeToEXRAM)
		ntram[getNTBank(bank) +addr] =val;
	else
		writePPU(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: reg) c =0;
		irqCounter =0;
		
		qtByte =0;
		lastNTAddr =0;
		isSprite =false;
	}
	sync();
	readCart =EMU->GetCPUReadHandler(0xD);
	EMU->SetCPUReadHandler(0xD, readReg);
	EMU->SetCPUWriteHandler(0xD, writeReg);
	
	readPPU =EMU->GetPPUReadHandler(0x0);
	writePPU =EMU->GetPPUWriteHandler(0x0);
	for (int bank =0x0; bank<=0x7; bank++) {
		EMU->SetPPUReadHandler(bank, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, readPPU);
	}
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetPPUReadHandler(bank, interceptNTRead);
		EMU->SetPPUReadHandlerDebug(bank, readPPU);
		EMU->SetPPUWriteHandler(bank, interceptNTWrite);
	}
}

void	MAPINT	cpuCycle (void) {
	if (irqEnabled && !++irqCounter) {
		EMU->SetIRQ(0);
		irqCounter =reg[CNTLO] | (reg[CNTHI] <<8);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: ntram) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	SAVELOAD_BYTE(stateMode, offset, data, qtByte);
	SAVELOAD_WORD(stateMode, offset, data, lastNTAddr);
	SAVELOAD_BOOL(stateMode, offset, data, isSprite);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =547;
} // namespace

MapperInfo MapperInfo_547 ={
	&mapperNum,
	_T("Konami Q太"),
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