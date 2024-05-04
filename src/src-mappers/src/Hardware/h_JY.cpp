#include	"h_JY.h"

namespace JY {
FSync		sync;
bool		allowExtendedMirroring;
uint8_t		mode;
uint8_t		ciramConfig;
uint8_t		vramConfig;
uint8_t		outerBank;

uint8_t		irqControl;
bool		irqEnabled;
uint8_t		irqPrescaler;
uint8_t		irqCounter;
uint8_t		irqXor;
uint8_t		lastA12;

uint8_t		prg[4];
uint16_t	chr[8];
uint16_t	nt[4];
uint8_t		latch[2];
uint8_t		mul1;
uint8_t		mul2;
uint8_t		adder;
uint8_t		test;

FCPUWrite	cpuWrite[0x10];
FPPURead	ppuRead[0x10];

#define	prgMode              (mode &0x03)
#define	switchableLastBank !!(mode &0x04)
#define chrMode              ((mode >>3) &0x03)
#define vromEnabled        !!(mode &0x20)
#define vromEverywhere     !!(mode &0x40)
#define romAt6000          !!(mode &0x80)
#define mirroring            (ciramConfig &0x03)
#define extendedMirroring  !!(ciramConfig &0x08)
#define chrWritable        !!(vramConfig &0x40)
#define vromBit            !!(vramConfig &0x80)
#define mmc4Mode           !!(outerBank &0x80)
#define irqSource            (irqControl &0x03)
#define smallPrescaler     !!(irqControl &0x04)
#define notCounting        !!(irqControl &0x08)
#define irqDirection         (irqControl >>6)

#define CHR8K         0
#define CHR4K         1
#define CHR2K         2
#define CHR1K         3
#define IRQ_M2        0
#define IRQ_PA12      1
#define IRQ_PPU_READ  2
#define IRQ_CPU_WRITE 3
#define IRQ_INCREASE  1
#define IRQ_DECREASE  2

uint8_t rev (uint8_t val) {
	return val <<6 &0x40 | val <<4 &0x20 | val <<2 &0x10 | val &0x08 | val >>2 &0x04 | val >>4 &0x02 | val >>6 &0x01;
}

void	syncPRG (int AND, int OR) {
	uint8_t prg3 =switchableLastBank? prg[3]: 0xFF;
	uint8_t prg6000 =0;
	switch (prgMode) {
		case 0:	EMU->SetPRG_ROM32(0x8,     prg3    &AND >>2 |OR >>2);
			prg6000 =prg[3] <<2 |3;
			break;
		case 1: EMU->SetPRG_ROM16(0x8,     prg[1]  &AND >>1 |OR >>1);
			EMU->SetPRG_ROM16(0xC,     prg3    &AND >>1 |OR >>1);
			prg6000 =prg[3] <<1 |1;
			break;
		case 2: EMU->SetPRG_ROM8 (0x8,     prg[0]  &AND     |OR    );
			EMU->SetPRG_ROM8 (0xA,     prg[1]  &AND     |OR    );
			EMU->SetPRG_ROM8 (0xC,     prg[2]  &AND     |OR    );
			EMU->SetPRG_ROM8 (0xE,       prg3  &AND     |OR    );
			prg6000 =prg[3]       ;
			break;
		case 3: EMU->SetPRG_ROM8 (0x8, rev(prg[0]) &AND     |OR    );
			EMU->SetPRG_ROM8 (0xA, rev(prg[1]) &AND     |OR    );
			EMU->SetPRG_ROM8 (0xC, rev(prg[2]) &AND     |OR    );
			EMU->SetPRG_ROM8 (0xE, rev(  prg3) &AND     |OR    );
			prg6000 =rev(prg[3])  ;
			break;
	}
	if (romAt6000)
		EMU->SetPRG_ROM8(0x6, prg6000 &AND |OR);
	else
		EMU->SetPRG_RAM8(0x6, 0); // Will be Open Bus if no RAM defined in NES 2.0 header
}

void	syncCHR (int and, int or) {
	switch(chrMode) {
		case CHR8K:                                             iNES_SetCHR_Auto8(0         , chr[            0 ] &and >>3 | or >>3); break;
		case CHR4K: for (int chrBank =0; chrBank <2; chrBank++) iNES_SetCHR_Auto4(chrBank *4, chr[latch[chrBank]] &and >>2 | or >>2); break;
		case CHR2K: for (int chrBank =0; chrBank <4; chrBank++) iNES_SetCHR_Auto2(chrBank *2, chr[     chrBank*2] &and >>1 | or >>1); break;
		case CHR1K: for (int chrBank =0; chrBank <8; chrBank++) iNES_SetCHR_Auto1(chrBank   , chr[     chrBank  ] &and     | or    ); break;
	}
	for (int i =0; i <8; i++) EMU->SetCHR_Ptr1(i, EMU->GetCHR_Ptr1(i), chrWritable);
}

void	syncNT (int and, int or) {
	if (vromEnabled) {
		for (int ntBank =0; ntBank <4; ntBank++) {
			int vromHere =!!(nt[ntBank] &0x80) ^vromBit |vromEverywhere;
			if (vromHere) {
				iNES_SetCHR_Auto1(ntBank +0x8, (nt[ntBank] &and) | or);
				iNES_SetCHR_Auto1(ntBank +0xC, (nt[ntBank] &and) | or);
			} else {
				EMU->SetCHR_NT1(ntBank +0x8, nt[ntBank] &1);
				EMU->SetCHR_NT1(ntBank +0xC, nt[ntBank] &1);
			}
		}
	} else if (extendedMirroring) {
		for (int ntBank =0; ntBank <4; ntBank++) {
			EMU->SetCHR_NT1(ntBank +0x8, nt[ntBank] &1);
			EMU->SetCHR_NT1(ntBank +0xC, nt[ntBank] &1);
		}
	} else {
		switch (mirroring) {
			case 0:	EMU->Mirror_V(); break;
			case 1:	EMU->Mirror_H(); break;
			case 2:	EMU->Mirror_S0(); break;
			case 3:	EMU->Mirror_S1(); break;
		}
	}
}

void	clockIRQ (void) {
	uint8_t mask =smallPrescaler? 0x07: 0xFF;
	if (irqEnabled) switch (irqDirection) {
		case IRQ_INCREASE:
			irqPrescaler =(irqPrescaler &~mask) | (++irqPrescaler &mask);
			if ((irqPrescaler &mask) ==0x00 && (notCounting? irqCounter: ++irqCounter) ==0x00) EMU->SetIRQ(0);
			break;
		case IRQ_DECREASE:
			irqPrescaler =(irqPrescaler &~mask) | (--irqPrescaler &mask);
			if ((irqPrescaler &mask) ==mask && (notCounting? irqCounter: --irqCounter) ==0xFF) EMU->SetIRQ(0);
			break;
	}
}

void	MAPINT	trapCPUWrite (int bank, int addr, int val) {
	if (irqSource ==IRQ_CPU_WRITE) clockIRQ();
	cpuWrite[bank](bank, addr, val);
}

int	MAPINT	trapPPURead (int bank, int addr) {
	if (irqSource ==IRQ_PPU_READ) clockIRQ();
	int result =ppuRead[bank](bank, addr);	// Any bankswitch will affect the next read, not this one.
	if (mmc4Mode && (bank &3) ==3) {
		int latchNumber =(bank &4) >>2;
		switch (addr &0x3F8) {
			case 0x3D8: latch[latchNumber] =(bank &4) |0; break;
			case 0x3E8: latch[latchNumber] =(bank &4) |2; break;
		}
		if (chrMode ==CHR4K) sync();
	}
	return result;
}

int	MAPINT	readMisc (int bank, int addr) {
	int result =*EMU->OpenBus;
	if ((addr &0x3FF) ==0 && addr !=0x800)
		result =(ROM->dipValue &0xC0) | (*EMU->OpenBus &0x3F);
	else
	if (addr &0x800) switch (addr &3) {
		case 0:	result =(mul1 *mul2) &0xFF;
			break;
		case 1:	result =(mul1 *mul2) >>8;
			break;
		case 2:	result =adder;
			break;
		case 3:	result =test;
			break;
	}
	return result;
}

void	MAPINT	writeMisc (int bank, int addr, int val) {
	switch (addr &3) {
		case 0:	mul1 =val;
			break;
		case 1:	mul2 =val;
			break;
		case 2:	adder +=val;
			break;
		case 3:	test =val;
			adder =0;
			break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	if (addr &0x800) return;
	prg[addr &3] = val;
	sync();
}

void	MAPINT	writeCHRLow (int bank, int addr, int val) {
	if (addr &0x800) return;
	chr[addr &7] =chr[addr &7] &0xFF00 | val;
	sync();
}

void	MAPINT	writeCHRHigh (int bank, int addr, int val) {
	if (addr &0x800) return;
	chr[addr &7] =chr[addr &7] &0x00FF | val <<8;
	sync();
}

void	MAPINT	writeNTBank (int bank, int addr, int val) {
	if (addr &0x800) return;
	if (~addr &4)
		nt[addr &3] =nt[addr &3] &0xFF00 | val;
	else
		nt[addr &3] =nt[addr &3] &0x00FF | val <<8;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val){
	switch (addr &7) {
		case 0:	irqEnabled =val &1;
			if (!irqEnabled) {
				irqPrescaler =0;
				EMU->SetIRQ(1);
			}
			break;
		case 1: irqControl =val;
			break;
		case 2:	irqEnabled =0;
			irqPrescaler =0;
			EMU->SetIRQ(1);
			break;
		case 3:	irqEnabled =1;
			break;
		case 4:	irqPrescaler =val ^irqXor;
			break;
		case 5:	irqCounter =val ^irqXor;
			break;
		case 6:	irqXor =val;
			break;
	}
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	if (addr &0x800) return;
	switch (addr &3) {
		case 0:	mode =val;
			if (!allowExtendedMirroring) mode &=~0x20;
			break;
		case 1:	ciramConfig =val;
			if (!allowExtendedMirroring) ciramConfig &=~0x08;
			break;
		case 2: vramConfig =val;
			break;
		case 3:	outerBank =val;
			break;
	}
	sync();
}

BOOL	MAPINT	load (FSync _sync, bool _allowExtendedMirroring) {
	sync =_sync;
	allowExtendedMirroring =_allowExtendedMirroring;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		mode         =0x00;
		ciramConfig  =0x00;
		vramConfig   =0x00;
		outerBank    =0x00;
		irqEnabled   =false;
		irqControl   =0x00;
		irqPrescaler =0x00;
		irqCounter   =0x00;
		irqXor       =0x00;
		lastA12      =0x00;
		mul1         =0x00;
		mul2         =0x00;
		adder        =0x00;
		test         =0x00;
		for (auto& c: prg) c =0;
		for (auto& c: chr) c =0;
		for (auto& c: nt ) c =0;
	}
	latch[0] =0;
	latch[1] =4;
	EMU->SetIRQ(1);
	sync();

	EMU->SetCPUReadHandler(0x5, readMisc);
	EMU->SetCPUWriteHandler(0x5, writeMisc);
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeCHRLow);
	EMU->SetCPUWriteHandler(0xA, writeCHRHigh);
	EMU->SetCPUWriteHandler(0xB, writeNTBank);
	EMU->SetCPUWriteHandler(0xC, writeIRQ);
	EMU->SetCPUWriteHandler(0xD, writeMode);
	// Trap CPU Writes and PPU Reads as sources for the IRQ clock.
	for (int bank =0x0; bank<=0xF; bank++) {
		cpuWrite[bank] =EMU->GetCPUWriteHandler(bank);
		EMU->SetCPUWriteHandler(bank, trapCPUWrite);

		ppuRead[bank] = EMU->GetPPUReadHandler(bank);
		EMU->SetPPUReadHandler(bank, trapPPURead);
		EMU->SetPPUReadHandlerDebug(bank, ppuRead[bank]);
	}
}

void	MAPINT	cpuCycle (void) {
	if (irqSource ==IRQ_M2) clockIRQ();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000 && !lastA12 && irqSource ==IRQ_PA12) clockIRQ();
	lastA12 =!!(addr &0x1000);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	SAVELOAD_BYTE(stateMode, offset, data, ciramConfig);
	SAVELOAD_BYTE(stateMode, offset, data, vramConfig);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, mul1);
	SAVELOAD_BYTE(stateMode, offset, data, mul2);
	SAVELOAD_BYTE(stateMode, offset, data, adder);
	SAVELOAD_BYTE(stateMode, offset, data, test);
	for (auto& c: latch) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: prg)   SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr)   SAVELOAD_WORD(stateMode, offset, data, c);
	for (auto& c: nt)    SAVELOAD_WORD(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, irqControl);
	SAVELOAD_BYTE(stateMode, offset, data, irqPrescaler);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BYTE(stateMode, offset, data, irqXor);
	SAVELOAD_BYTE(stateMode, offset, data, lastA12);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace