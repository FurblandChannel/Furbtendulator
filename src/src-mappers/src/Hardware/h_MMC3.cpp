#include	"h_MMC3.h"

namespace MMC3 {
MMC3Type	mmc3Type;
uint8_t		pointer;
uint8_t		reg[8];
uint8_t		mirroring;
uint8_t		wram;

uint8_t		counter;
uint8_t		prescaler;
uint8_t		reloadValue;
bool		reload;
bool		enableIRQ;
uint8_t		pa12Filter;
uint16_t	prevAddr;
bool		counterUnderflow;

FSync		sync;
FCPURead	wramRead, wramReadCallback;
FCPUWrite	wramWrite, wramWriteCallback;

#define	prgInvert     (pointer &0x40)
#define	chrInvert     (pointer &0x80)
#define	wramEnabled   (wram &0x80)
#define	wramProtected (wram &0x40)

void	syncWRAM (void) {
	if (wramEnabled) {
		EMU->SetPRG_RAM8(0x6, 0);
		if (wramProtected) {
			EMU->SetPRG_Ptr4(0x6, EMU->GetPRG_Ptr4(0x6), FALSE);
			EMU->SetPRG_Ptr4(0x7, EMU->GetPRG_Ptr4(0x7), FALSE);
		}
	} else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
}

int	getPRGBank (int bank) {
	if (~bank &1 && prgInvert) bank ^=2;
	return bank &2? 0xFE | bank &1: reg[6 | bank &1];
};

void	syncPRG (int AND, int OR) {
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, getPRGBank(bank) &AND |OR);
}

void	syncPRG_NROM (int AND, int OR, int bankAND, int cpuMask) {
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, getPRGBank(bank &bankAND) &AND &~cpuMask|OR |bank &cpuMask);
}

void	syncPRG_GNROM_66 (int A14, int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, (getPRGBank(0) &~A14 &~1) &AND |OR);
	EMU->SetPRG_ROM8(0xA, (getPRGBank(0) &~A14 | 1) &AND |OR);
	EMU->SetPRG_ROM8(0xC, (getPRGBank(0) | A14 &~1) &AND |OR);
	EMU->SetPRG_ROM8(0xE, (getPRGBank(0) | A14 | 1) &AND |OR);
}

void	syncPRG_GNROM_67 (int A14, int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, (getPRGBank(0) &~A14) &AND |OR);
	EMU->SetPRG_ROM8(0xA, (getPRGBank(1) &~A14) &AND |OR);
	EMU->SetPRG_ROM8(0xC, (getPRGBank(0) | A14) &AND |OR);
	EMU->SetPRG_ROM8(0xE, (getPRGBank(1) | A14) &AND |OR);
}

void	syncPRG_2 (int AND8, int ANDC, int OR8, int ORC) {
	EMU->SetPRG_ROM8(0x8, getPRGBank(0) &AND8 |OR8);
	EMU->SetPRG_ROM8(0xA, getPRGBank(1) &AND8 |OR8);
	EMU->SetPRG_ROM8(0xC, getPRGBank(2) &ANDC |ORC);
	EMU->SetPRG_ROM8(0xE, getPRGBank(3) &ANDC |ORC);
}

void	syncPRG_4 (int AND8, int ANDA, int ANDC, int ANDE, int OR8, int ORA, int ORC, int ORE) {
	EMU->SetPRG_ROM8(0x8, getPRGBank(0) &AND8 |OR8);
	EMU->SetPRG_ROM8(0xA, getPRGBank(1) &ANDA |ORA);
	EMU->SetPRG_ROM8(0xC, getPRGBank(2) &ANDC |ORC);
	EMU->SetPRG_ROM8(0xE, getPRGBank(3) &ANDE |ORE);
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

void	syncCHR_ROM (int AND, int OR0, int OR4) {
	for (int bank =0; bank <4; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR0);
	for (int bank =4; bank <8; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR4);
}

void	syncCHR_ROM (int AND, int OR0, int OR2, int OR4, int OR6) {
	for (int bank =0; bank <2; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR0);
	for (int bank =2; bank <4; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR2);
	for (int bank =4; bank <6; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR4);
	for (int bank =6; bank <8; bank++) EMU->SetCHR_ROM1(bank, getCHRBank(bank) &AND |OR6);
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

void	syncMirrorA17 (void) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_NT1(bank |8, getCHRBank(bank) >>7);
}

int	MAPINT	trapWRAMRead (int bank, int addr) {
	if (wramEnabled)
		return wramReadCallback(bank, addr);
	else
		return wramRead(bank, addr);
}

void	MAPINT	trapWRAMWrite (int bank, int addr, int val) {
	if (wramEnabled && !wramProtected)
		wramWriteCallback(bank, addr, val);
	else
		wramWrite(bank, addr, val);
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
	if (addr &1) {
		counter =0;
		prescaler =7;
		reload =true;
	} else
		reloadValue =val;
}

void	MAPINT	writeIRQEnable (int bank, int addr, int val) {
	enableIRQ =!!(addr &1);
	if (!enableIRQ) EMU->SetIRQ(1);
}

void	MAPINT	load (FSync _sync) {
	load (_sync, MMC3Type::Sharp);
}

void	MAPINT	load (FSync _sync, MMC3Type _mmc3Type) {
	sync =_sync;
	mmc3Type =_mmc3Type;
}

void	setWRAMCallback (FCPURead theWRAMReadCallback, FCPUWrite theWRAMWriteCallback) {
	wramReadCallback  =theWRAMReadCallback?  theWRAMReadCallback:  wramRead;
	wramWriteCallback =theWRAMWriteCallback? theWRAMWriteCallback: wramWrite;
	sync();
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
		reload =false;
		counter =0;
		prescaler =7;
		pa12Filter =0;
		prevAddr =0;
	}
	wramRead  =wramReadCallback  =EMU->GetCPUReadHandler(0x6);
	wramWrite =wramWriteCallback =EMU->GetCPUWriteHandler(0x6);
	for (int bank =0x6; bank<=0x7; bank++) {
		EMU->SetCPUReadHandler(bank, trapWRAMRead);
		EMU->SetCPUReadHandlerDebug(bank, trapWRAMRead);
		EMU->SetCPUWriteHandler(bank, trapWRAMWrite);
	}
	
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
	switch (mmc3Type) {
		default:
		case MMC3Type::NEC:
			ppuCycle_NEC (addr, scanline, cycle, isRendering);
			break;
		case MMC3Type::Sharp:
			ppuCycle_Sharp (addr, scanline, cycle, isRendering);
			break;
		case MMC3Type::Acclaim:
			ppuCycle_Acclaim (addr, scanline, cycle, isRendering);
			break;
	}
}

void	MAPINT	ppuCycle_NEC (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			uint8_t prevCounter =counter;
			if (!counter || reload)
				counter =reloadValue;
			else
				counter--;
			if (!counter && (prevCounter || reload) && enableIRQ)
				EMU->SetIRQ(0);
			reload =false;
		}
		pa12Filter =5;
	}
}

void	MAPINT	ppuCycle_Sharp (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			if (!counter || reload)
				counter =reloadValue;
			else
				counter--;
			if (!counter && enableIRQ)
				EMU->SetIRQ(0);
			reload =false;
		}
		pa12Filter =3;
	}
}

void	MAPINT	ppuCycle_Acclaim (int addr, int scanline, int cycle, int isRendering) {
	if (prevAddr &0x1000 && ~addr &0x1000) {
		if (!(prescaler++ &7)) {
			if (!counter || reload)
				counter =reloadValue;
			else
				counter--;
			if (!counter && enableIRQ)
				EMU->SetIRQ(0);
			reload =false;
		}
	}
	prevAddr =addr;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, pointer);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, wram);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, prescaler);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BOOL(stateMode, offset, data, reload);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_WORD(stateMode, offset, data, prevAddr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
} // namespace MMC3