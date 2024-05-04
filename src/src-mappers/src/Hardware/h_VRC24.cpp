#include	"h_VRC24.h"

namespace VRC24 {
FSync		sync;
FCPUWrite	writePort;
bool		allowIRQContinue;
uint8_t		irqDelay;
int		A0, A1;
bool		vrc4;

uint8_t		irq;
uint8_t		counter;
uint8_t 	latch;
int16_t		cycles;
uint8_t		prgFlip;
bool		wramEnable;
uint8_t 	wires;

uint8_t		prg[2];
uint16_t	chr[8];
uint8_t		mirroring;
uint8_t		irqRaiseCount;


void	MAPINT	syncPRG (int AND, int OR) {
	if (wramEnable || !vrc4)
		EMU->SetPRG_RAM8(0x6, 0);
	else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
	EMU->SetPRG_ROM8(0x8 ^prgFlip, prg[0] &AND |OR);
	EMU->SetPRG_ROM8(0xA         , prg[1] &AND |OR);
	EMU->SetPRG_ROM8(0xC ^prgFlip, 0xFE   &AND |OR);
	EMU->SetPRG_ROM8(0xE         , 0xFF   &AND |OR);
}

void	MAPINT	syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize)
		syncCHR_ROM(AND, OR);
	else
		syncCHR_RAM(AND, OR);
}

void	MAPINT	syncCHR_ROM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank] &AND |OR);
}

void	MAPINT	syncCHR_ROM (int AND, int OR0, int OR2, int OR4, int OR6) {
	EMU->SetCHR_ROM1(0x0, chr[0] &AND |OR0);
	EMU->SetCHR_ROM1(0x1, chr[1] &AND |OR0);
	EMU->SetCHR_ROM1(0x2, chr[2] &AND |OR2);
	EMU->SetCHR_ROM1(0x3, chr[3] &AND |OR2);
	EMU->SetCHR_ROM1(0x4, chr[4] &AND |OR4);
	EMU->SetCHR_ROM1(0x5, chr[5] &AND |OR4);
	EMU->SetCHR_ROM1(0x6, chr[6] &AND |OR6);
	EMU->SetCHR_ROM1(0x7, chr[7] &AND |OR6);
}

void	MAPINT	syncCHR_RAM (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_RAM1(bank, chr[bank] &AND |OR);
}

void	MAPINT	syncMirror() {
	switch (mirroring &(vrc4? 3: 1)) {
		case 0:	EMU->Mirror_V (); break;
		case 1:	EMU->Mirror_H (); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	write (int bank, int addr, int val) {
	switch(bank) {
		case 0x8: case 0xA:
			writePRG(bank, addr, val);
			break;
		case 0x9:
			writeMisc(bank, addr, val);
			break;
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
			writeCHR(bank, addr, val);
		        break;
		case 0xF:
			if (vrc4) writeIRQ(bank, addr, val);
		        break;
	}
}

int	MAPINT	readWRAM (int bank, int addr) {
	if (ROM->PRGRAMSize && ROM->PRGRAMSize <8192) {
		bank &=~1;
		addr &=((64 << ((ROM->INES2_PRGRAM &0xF0)? ROM->INES2_PRGRAM >>4: ROM->INES2_PRGRAM &0x0F)) -1);
	}
	return EMU->ReadPRG(bank, addr);
}

void	MAPINT	writeWRAM (int bank, int addr, int val) {
	if (ROM->PRGRAMSize && ROM->PRGRAMSize <8192) {
		bank &=~1;
		addr &=((64 << ((ROM->INES2_PRGRAM &0xF0)? ROM->INES2_PRGRAM >>4: ROM->INES2_PRGRAM &0x0F)) -1);
	}
	EMU->WritePRG(bank, addr, val);
}

int	MAPINT	readWires (int bank, int addr) {
	return *EMU->OpenBus &~1 | wires >>3 &1;
}

void	MAPINT	writeWires (int bank, int addr, int val) {
	wires =wires &~7 | val &7;
	sync();
}
void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[bank >>1 &1] =val;
	sync();
}

void	MAPINT	writeMisc (int bank, int addr, int val) {
	int reg =(addr &A1? 2: 0) | (addr &A0? 1: 0);
	switch (reg &(vrc4? 3: 0)) {
		case 0:
		case 1:	mirroring =val &3;
			break;
		case 2:	wramEnable =!!(val &1);
			prgFlip =val &2? 4: 0;
			break;
		case 3:	if (writePort) writePort(bank, addr, val);
			break;
	}
	sync();
}

void	MAPINT  writeCHR (int bank, int addr, int val) {
	int reg =((bank -0xB)<<1) | (addr &A1? 1: 0);
	if (addr &A0)
		chr[reg] =chr[reg] &0x00F | val <<4;
	else
		chr[reg] =chr[reg] &0xFF0 | val &0xF;
	sync();	
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	int reg =(addr &A1? 2: 0) | (addr &A0? 1: 0);
	switch (reg) {
		case 0:	latch =latch &0xF0 | val &0xF;
			break;
		case 1:	latch =latch &0x0F | val <<4;
			break;
		case 2:	irq =val;
			if (irq &2) {
				counter =latch;
				cycles =341;
			}
			EMU->SetIRQ(1);
			break;
		case 3:	if (allowIRQContinue) irq =irq &~2 | irq <<1 &2;
			EMU->SetIRQ(1);
			break;
	}
}

void	MAPINT	load (FSync _sync, bool _vrc4, int _A0, int _A1, FCPUWrite _writePort, bool _allowIRQContinue, uint8_t _irqDelay) {
	sync =_sync;
	vrc4 =_vrc4;
	A0 =_A0;
	A1 =_A1;
	writePort =_writePort;
	allowIRQContinue =_allowIRQContinue;
	irqDelay =_irqDelay;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		irq =0;
		counter =0;
		latch =0;
		wires =0;
		cycles =0;
		prgFlip =0;
		prg[0] =0;
		prg[1] =1;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		wramEnable =ROM->INES2_SubMapper >1? false: true; // Always enable WRAM on NES 1.0 or unknown chip type
	}
	EMU->SetIRQ(1);
	sync();

	// For small WRAM, address wrapping needs to be implemented manually
	if (vrc4) {
		for (int bank =0x6; bank <=0x7; bank++) {
			EMU->SetCPUReadHandler     (bank, readWRAM);
			EMU->SetCPUReadHandlerDebug(bank, readWRAM);
			EMU->SetCPUWriteHandler    (bank, writeWRAM);
		}
		EMU->SetCPUWriteHandler(0xF, writeIRQ);
	} else
	if (ROM->INES_Version >=2 && ROM->INES2_PRGRAM ==0) {
		EMU->SetCPUReadHandler(0x6, readWires);
		EMU->SetCPUReadHandlerDebug(0x6, readWires);
		EMU->SetCPUWriteHandler(0x6, writeWires);
	}
		
	for (int bank =0x8; bank <=0xA; bank +=2) EMU->SetCPUWriteHandler(bank, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeMisc);
	for (int bank =0xB; bank<=0xE; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
}

void	MAPINT	cpuCycle (void) {
	if (irqRaiseCount && !--irqRaiseCount) EMU->SetIRQ(0);
	if (irq &2 && (irq &4 || (cycles -= 3) <=0)) {
		if (~irq &4) cycles +=341;
		if (!++counter) {
			counter =latch;
			irqRaiseCount =irqDelay;
			if (!irqRaiseCount) EMU->SetIRQ(0);
		}
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_WORD(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (vrc4) {
		SAVELOAD_BYTE(stateMode, offset, data, irq);
		SAVELOAD_BYTE(stateMode, offset, data, counter);
		SAVELOAD_BYTE(stateMode, offset, data, latch);
		SAVELOAD_WORD(stateMode, offset, data, cycles);
		SAVELOAD_BYTE(stateMode, offset, data, prgFlip);
		SAVELOAD_BYTE(stateMode, offset, data, irqRaiseCount);
		SAVELOAD_BOOL(stateMode, offset, data, wramEnable);
	} else
		SAVELOAD_BYTE(stateMode, offset, data, wires);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace VRC24
