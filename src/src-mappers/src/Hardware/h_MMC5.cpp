#include	"h_MMC5.h"
#include	"..\Hardware\Sound\s_MMC5.h"

namespace MMC5 {
void		(MAPINT *SetCHR_Auto1)	(int,int);
void		(MAPINT *SetCHR_Auto2)	(int,int);
void		(MAPINT *SetCHR_Auto4)	(int,int);
void		(MAPINT *SetCHR_Auto8)	(int,int);
FCPURead	readPPU;
FCPUWrite	writePPU;
FCPURead	readBankF;
FPPURead	readCHR;
FPPURead	readNT;

bool		sprites8x16;
bool		ppuRendering;
bool		fetchingSprites;

uint8_t		prgMode;
uint8_t		prg[5];
uint8_t		wramProtect1, wramProtect2;

uint8_t		chrMode, chrMSB, lastCHRReg;
uint16_t	chr[12];
uint8_t		nametables;

// EXRAM
uint8_t		extMode;
uint8_t		extRAM[1024];
uint8_t		fillData[1024];
uint8_t		extCHRBank;
uint16_t	lastNTAddr;

// IRQ
uint8_t		irqTarget, irqScanline;
bool		irqEnabled;
bool		irqPending;

// Vertical split
bool		splitActive;
bool		splitFineY;
uint8_t		split;
uint8_t		splitScroll;
uint8_t		splitBank;
uint8_t		splitScanline;
uint16_t	splitAddr;

// Scanline detector
bool		inFrame;
bool		ppuIsReading;
uint16_t	lastPPUReadAddr;
uint8_t		ppuReadAddrMatches;
uint8_t		ntReadCount;
uint8_t		ppuIdleCount;

// Miscellaneous
uint8_t		multiplier1, multiplier2;
bool		timerRunning;
bool		timerIRQ;
uint16_t	timerCount;

static const uint8_t
	EXT_NT =0,
	EXT_ATTR =1,
	EXT_WRAM =2,
	EXT_WROM =3
;
#define	splitEnabled   !!(split &0x80)
#define	splitRightSide !!(split &0x40)
#define	splitTile      (split &0x1F)

void	syncPRG (void) {
	EMU->SetPRG_RAM8(0x6, prg[0]);
	switch(prgMode) {
		case 0:	                  EMU->SetPRG_ROM32(0x8, prg[4] >>2);
			break;
		case 1:	if (prg[2] &0x80) EMU->SetPRG_ROM16(0x8, prg[2] >>1); else EMU->SetPRG_RAM16(0x8, prg[2] >>1);
			                  EMU->SetPRG_ROM16(0xC, prg[4] >>1);
			break;
		case 2:	if (prg[2] &0x80) EMU->SetPRG_ROM16(0x8, prg[2] >>1); else EMU->SetPRG_RAM16(0x8, prg[2] >>1);
			if (prg[3] &0x80) EMU->SetPRG_ROM8 (0xC, prg[3] >>0); else EMU->SetPRG_RAM8 (0xC, prg[3] >>0);
			                  EMU->SetPRG_ROM8 (0xE, prg[4] >>0);
			break;
		case 3:	if (prg[1] &0x80) EMU->SetPRG_ROM8 (0x8, prg[1] >>0); else EMU->SetPRG_RAM8 (0x8, prg[1] >>0);
			if (prg[2] &0x80) EMU->SetPRG_ROM8 (0xA, prg[2] >>0); else EMU->SetPRG_RAM8 (0xA, prg[2] >>0);
			if (prg[3] &0x80) EMU->SetPRG_ROM8 (0xC, prg[3] >>0); else EMU->SetPRG_RAM8 (0xC, prg[3] >>0);
			                  EMU->SetPRG_ROM8 (0xE, prg[4] >>0);
			break;
	}
	if (wramProtect1 !=2 || wramProtect2 !=1) for (int bank =0x6; bank<=0xF; bank++) EMU->SetPRG_Ptr4(bank, (EMU->GetPRG_Ptr4(bank)), FALSE);
}

void	syncCHR (void) {
	if (inFrame && splitActive && !fetchingSprites) {
		SetCHR_Auto4(0x0, splitBank);
		SetCHR_Auto4(0x4, splitBank);
	} else
	if (inFrame && extMode ==EXT_ATTR && !fetchingSprites) {
		/*if (ROM->PRGROMCRC32 ==0x1C79304E) { // Demo Boy
			if (extCHRBank &4) {
				EMU->SetCHR_ROM4(0x0, extCHRBank);
				EMU->SetCHR_ROM4(0x4, extCHRBank);
			} else {
				EMU->SetCHR_RAM4(0x0, extCHRBank);
				EMU->SetCHR_RAM4(0x4, extCHRBank);
			}
		} else {*/
			SetCHR_Auto4(0x0, extCHRBank);
			SetCHR_Auto4(0x4, extCHRBank);
		//}
	} else
	if (!sprites8x16 || fetchingSprites || !inFrame && lastCHRReg <8) {
		switch(chrMode) {
			case 0:	SetCHR_Auto8(0x0, chr[7]);
				break;
			case 1:	SetCHR_Auto4(0x0, chr[3]);
				SetCHR_Auto4(0x4, chr[7]);
				break;
			case 2:	SetCHR_Auto2(0x0, chr[1]);
				SetCHR_Auto2(0x2, chr[3]);
				SetCHR_Auto2(0x4, chr[5]);
				SetCHR_Auto2(0x6, chr[7]);
				break;
			case 3:	SetCHR_Auto1(0x0, chr[0]);
				SetCHR_Auto1(0x1, chr[1]);
				SetCHR_Auto1(0x2, chr[2]);
				SetCHR_Auto1(0x3, chr[3]);
				SetCHR_Auto1(0x4, chr[4]);
				SetCHR_Auto1(0x5, chr[5]);
				SetCHR_Auto1(0x6, chr[6]);
				SetCHR_Auto1(0x7, chr[7]);
				break;
		}
	} else { // BG in 8x16 sprite mode
		switch(chrMode) {
			case 0:	SetCHR_Auto8(0x0, chr[11]);
				break;
			case 1:	SetCHR_Auto4(0x0, chr[11]);
				SetCHR_Auto4(0x4, chr[11]);
				break;
			case 2:	SetCHR_Auto2(0x0, chr[9]);
				SetCHR_Auto2(0x2, chr[11]);
				SetCHR_Auto2(0x4, chr[9]);
				SetCHR_Auto2(0x6, chr[11]);
				break;
			case 3:	SetCHR_Auto1(0x0, chr[8]);
				SetCHR_Auto1(0x1, chr[9]);
				SetCHR_Auto1(0x2, chr[10]);
				SetCHR_Auto1(0x3, chr[11]);
				SetCHR_Auto1(0x4, chr[8]);
				SetCHR_Auto1(0x5, chr[9]);
				SetCHR_Auto1(0x6, chr[10]);
				SetCHR_Auto1(0x7, chr[11]);
				break;
		}
	}
}

void	syncNT (void) {
	for (int bank =0; bank <4; bank++) {
		switch ((nametables >>bank*2) &3) {
			case 0:	EMU->SetCHR_NT1 (bank +8, 0);
				EMU->SetCHR_NT1 (bank +12,0);
				break;
			case 1:	EMU->SetCHR_NT1 (bank +8, 1);
				EMU->SetCHR_NT1 (bank +12,1);
				break;
			case 2:	EMU->SetCHR_Ptr1(bank +8, extRAM, true);
				EMU->SetCHR_Ptr1(bank +12,extRAM, true);
				break;
			case 3:	EMU->SetCHR_Ptr1(bank +8, fillData, false);
				EMU->SetCHR_Ptr1(bank +12,fillData, false);
				break;
		}
	}
}

void	clearInFrame (bool resetIRQ) {
	if (inFrame || resetIRQ) {
		inFrame =false;
		lastPPUReadAddr =0xFFFF;
		syncCHR();
		if (resetIRQ) {
			irqPending =false;
			irqScanline =0;
		}
	}
}

void	checkIRQs (void) {
	bool irq =(MMC5sound::HaveIRQ() || irqPending && irqEnabled || timerIRQ);
	EMU->SetIRQ(irq? 0: 1);
}

int	MAPINT	interceptBankFRead (int bank, int addr) {
	if (addr ==0xFFA || addr ==0xFFB) clearInFrame(true);
	return readBankF(bank, addr);
}

void	MAPINT	interceptPPUWrite (int bank, int addr, int val) {
	switch(addr) {
		case 0:	sprites8x16 =!!(val &0x20);
			syncCHR();
			break;
		case 1:	ppuRendering =!!(val &0x18);
			if (!ppuRendering) clearInFrame(false);
			break;
	}
	writePPU(bank, addr, val);
}

void	scanlineDetector (int bank, int addr) {
	addr |=bank <<10;
	if (addr >=0x2000 && addr <=0x2FFF && addr ==lastPPUReadAddr)
		ppuReadAddrMatches++;
	else {
		if (ppuReadAddrMatches ==2) {
			if (!inFrame) {
				inFrame =true;
				fetchingSprites =false;
				irqScanline =0;
				syncCHR();
			} else
			if (++irqScanline ==irqTarget) {
				irqPending =true;
				checkIRQs();
			}
			if (irqScanline ==240) clearInFrame(true);
			ntReadCount =1;
			splitScanline =irqScanline +splitScroll;
			if (splitScanline >=240) splitScanline -=240;
		}
		ppuReadAddrMatches =0;
	}
	lastPPUReadAddr =addr;
	ppuIsReading =true;
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	scanlineDetector(bank, addr);

	// In SL mode, if in split region, apply fine Y scrolling
	if (splitFineY && splitActive) addr =(addr &~0x7) | (splitScanline &7);
	return readCHR(bank, addr);
}

int	MAPINT	interceptNTRead (int bank, int addr) {
	int	result =readNT(bank, addr);
	scanlineDetector(bank, addr);

	if (addr <0x3C0) {		// Nametable read
		lastNTAddr =addr;	// Needed for EXRAM value during subsequent attribute fetch

		if (inFrame) {
			// Two fetches per tile; the first tile after the two scanline-detector-triggering garbage NT reads is tile 2.
			// Tiles 42 and 43 are actually tiles 0 and 1. We could just query "currentTile ==42" instead of "==0", but then the split tile comparison would be off.
			uint8_t currentTile =((ntReadCount >>1) +2) %42;

			if (currentTile ==34) {
				fetchingSprites =true;
				syncCHR();
			} else
			if (currentTile ==0) {
				fetchingSprites =false;
				syncCHR();
			}

			// Evaluate whether we are entering/leaving the split region, and sync if so.
			bool newSplitActive =currentTile <splitTile;
			if (splitRightSide) newSplitActive =!newSplitActive;
			if (!splitEnabled || fetchingSprites) newSplitActive =false;
			if (splitActive !=newSplitActive) {
				splitActive =newSplitActive;
				syncCHR();
			}
			if (splitActive) {
				splitAddr =(currentTile &0x1F) | ((splitScanline &~0x07) <<2);
				result =extRAM[splitAddr &0x3FF];
			}
		}
	} else	// Attribute table read
	if (splitActive && inFrame)
		result =extRAM[0x3C0 | ((splitAddr &0x380) >>4) | ((splitAddr &0x1C) >>2)];
	else
	if (extMode ==EXT_ATTR) {
		extCHRBank =(extRAM[lastNTAddr] &0x3F) | (chrMSB <<6);
		syncCHR();
		uint8_t extAttrColor =extRAM[lastNTAddr] >>6;
		result =extAttrColor | (extAttrColor <<2) | (extAttrColor <<4) | (extAttrColor <<6);
	}
	ntReadCount++;
	return result;
}

int	MAPINT	readASIC (int bank, int addr) {
	int result;
	if (addr >=0x000 && addr <=0x0FF)
		return MMC5sound::Read((bank <<12) |addr);
	else
	if (addr >=0xC00 && addr <=0xFFF)
		return extRAM[addr -0xC00];
	else
	switch(addr) {
		case 0x204:	result =(irqPending? 0x80: 0x00) | (inFrame? 0x40: 0x00);
				irqPending =false;
				checkIRQs();
				return result;
		case 0x205:	return (multiplier1 *multiplier2) &0xFF;
		case 0x206:	return (multiplier1 *multiplier2) >>8;
		case 0x209:	result =(timerIRQ? 0x80: 0x00);
				timerIRQ =false;
				checkIRQs();
				return result;
		default:	return *EMU->OpenBus;
	}
}

int	MAPINT	readASICDebug (int bank, int addr) {
	if (addr >=0xC00 && addr <=0xFFF && extMode >=EXT_WRAM)
		return extRAM[addr -0xC00];
	else
	switch(addr) {
		case 0x204:	return (irqPending? 0x80: 0x00) | (inFrame? 0x40: 0x00);
		case 0x205:	return (multiplier1 *multiplier2) &0xFF;
		case 0x206:	return (multiplier1 *multiplier2) >>8;
		case 0x209:	return timerIRQ? 0x80: 0x00;
		default:	return *EMU->OpenBus;
	}
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (addr >=0x000 && addr <=0x0FF)
		MMC5sound::Write((bank <<12) |addr, val);
	else
	if (addr >=0xC00 && addr <=0xFFF)
		extRAM[addr -0xC00] =val;
	else
	if (addr >=0x113 && addr <=0x117) {
		if (~val &0x80 && ROM->PRGRAMSize ==16384) val >>=2;	// ETROM selects chips via bit 2
		prg[addr -0x113] =val;
		syncPRG();
	} else
	if (addr >=0x120 && addr <=0x12B) {
		chr[lastCHRReg =addr -0x120] =val | (chrMSB <<8);
		syncCHR();
	} else
	switch(addr) {
		case 0x100:	prgMode =val &3;
				syncPRG();
				break;
		case 0x101:	chrMode =val &3;
				syncCHR();
				break;
		case 0x102:	wramProtect1 =val &3;
				syncPRG();
				break;
		case 0x103:	wramProtect2 =val &3;
				syncPRG();
				break;
		case 0x104:	extMode =val &3;
				syncCHR();
				break;
		case 0x105:	nametables =val;
				syncNT();
				break;
		case 0x106:	for (int i =0x000; i <0x3C0; i++) fillData[i] =val;
				break;
		case 0x107:	val &=3;
				for (int i =0x3C0; i <0x400; i++) fillData[i] =val | (val <<2) | (val <<4) | (val <<6);
				break;
		case 0x130:	chrMSB =val &3;
				syncCHR();
				break;
		case 0x200:	split =val;
				if (!splitEnabled) splitActive =false;
				syncCHR();
				break;
		case 0x201:	splitScroll =val;
				break;
		case 0x202:	splitBank =val;
				break;
		case 0x203:	irqTarget =val;
				break;
		case 0x204:	irqEnabled =!!(val &0x80);
				checkIRQs();
				break;
		case 0x205:	multiplier1 =val;
				break;
		case 0x206:	multiplier2 =val;
				break;
		case 0x209:	timerCount =(timerCount &~0x00FF) | val;
				timerRunning =true;
				break;
		case 0x20A:	timerCount =(timerCount &~0xFF00) |(val <<8);
				break;
	}
}

void	MAPINT	load (void) {
	MMC5sound::Load();
	if (ROM->CHRROMSize) {
		SetCHR_Auto1 =EMU->SetCHR_ROM1;
		SetCHR_Auto2 =EMU->SetCHR_ROM2;
		SetCHR_Auto4 =EMU->SetCHR_ROM4;
		SetCHR_Auto8 =EMU->SetCHR_ROM8;
	} else {
		SetCHR_Auto1 =EMU->SetCHR_RAM1;
		SetCHR_Auto2 =EMU->SetCHR_RAM2;
		SetCHR_Auto4 =EMU->SetCHR_RAM4;
		SetCHR_Auto8 =EMU->SetCHR_RAM8;
	}
	ROM->ChipRAMSize =sizeof(extRAM);
	ROM->ChipRAMData =extRAM;
}

void	MAPINT	reset (RESET_TYPE resetType) {	// MMC5 detects reset, so make no distinction between hard and soft reset.
	sprites8x16 =false;
	ppuRendering =false;
	fetchingSprites =false;

	prgMode =3; // Koei games depend on this default value
	for (int i =0; i <5; i++) prg[i] =0xFB +i;
	wramProtect1 =wramProtect2 =0;

	chrMode =0;
	chrMSB =0;
	lastCHRReg =0;
	for (int i =0; i <12; i++) chr[i] =i;
	nametables =0;

	extMode =EXT_NT;
	//for (int i =0; i <1024; i++) extRAM[i] =0x00;
	for (int i =0; i <1024; i++) fillData[i] =0x00;
	extCHRBank =0;
	lastNTAddr =0;

	irqTarget =0;
	irqScanline =0;
	irqEnabled =false;
	irqPending =false;

	splitActive =false;
	splitFineY =ROM->INES2_SubMapper &1;
	split =0;
	splitScroll =0;
	splitBank =0;
	splitScanline =0;
	splitAddr =0;

	inFrame =false;
	ppuIsReading =false;
	lastPPUReadAddr =0;
	ppuReadAddrMatches =0;
	ntReadCount =0;
	ppuIdleCount =0;

	multiplier1 =0xFF;
	multiplier2 =0xFF;
	timerRunning =false;
	timerIRQ =false;
	timerCount =0;

	syncPRG();
	syncCHR();
	syncNT();
	MMC5sound::Reset(resetType);

	readPPU =EMU->GetCPUReadHandler(0x2);
	writePPU =EMU->GetCPUWriteHandler(0x2);
	readBankF =EMU->GetCPUReadHandler(0xF);
	readCHR =EMU->GetPPUReadHandler(0x0);
	readNT =EMU->GetPPUReadHandler(0x8);

	EMU->SetCPUWriteHandler(0x2, interceptPPUWrite);
	EMU->SetCPUReadHandler(0x5, readASIC);
	EMU->SetCPUReadHandlerDebug(0x5, readASICDebug);
	EMU->SetCPUWriteHandler(0x5, writeASIC);
	EMU->SetCPUReadHandler(0xF, interceptBankFRead);
	for (int i =0x0; i<=0x7; i++) {
		EMU->SetPPUReadHandler(i, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(i, readCHR);
	}
	for (int i =0x8; i<=0xF; i++) {
		EMU->SetPPUReadHandler(i, interceptNTRead);
		EMU->SetPPUReadHandlerDebug(i, readNT);
	}
}

void	MAPINT	cpuCycle (void) {
	if (ppuIsReading)
		ppuIdleCount =0;
	else
	if (++ppuIdleCount ==3)
		clearInFrame(false);
	ppuIsReading =false;
	
	if (timerRunning && !--timerCount) {
		timerIRQ =true;
		checkIRQs();
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, sprites8x16);
	SAVELOAD_BOOL(stateMode, offset, data, ppuRendering);
	SAVELOAD_BOOL(stateMode, offset, data, fetchingSprites);

	SAVELOAD_BYTE(stateMode, offset, data, prgMode);
	for (int i =0; i <5; i++) SAVELOAD_BYTE(stateMode, offset, data, prg[i]);
	SAVELOAD_BYTE(stateMode, offset, data, wramProtect1);
	SAVELOAD_BYTE(stateMode, offset, data, wramProtect2);

	SAVELOAD_BYTE(stateMode, offset, data, chrMode);
	SAVELOAD_BYTE(stateMode, offset, data, chrMSB);
	SAVELOAD_BYTE(stateMode, offset, data, lastCHRReg);
	for (int i =0; i <12; i++) SAVELOAD_WORD(stateMode, offset, data, chr[i]);
	SAVELOAD_BYTE(stateMode, offset, data, nametables);

	SAVELOAD_BYTE(stateMode, offset, data, extMode);
	for (int i =0; i <1024; i++) SAVELOAD_BYTE(stateMode, offset, data, extRAM[i]);
	for (int i =0; i <1024; i++) SAVELOAD_BYTE(stateMode, offset, data, fillData[i]);
	SAVELOAD_BYTE(stateMode, offset, data, extCHRBank);
	SAVELOAD_WORD(stateMode, offset, data, lastNTAddr);

	SAVELOAD_BYTE(stateMode, offset, data, irqTarget);
	SAVELOAD_BYTE(stateMode, offset, data, irqScanline);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	SAVELOAD_BOOL(stateMode, offset, data, irqPending);

	SAVELOAD_BOOL(stateMode, offset, data, splitActive);
	SAVELOAD_BOOL(stateMode, offset, data, splitFineY);
	SAVELOAD_BYTE(stateMode, offset, data, split);
	SAVELOAD_BYTE(stateMode, offset, data, splitScroll);
	SAVELOAD_BYTE(stateMode, offset, data, splitBank);
	SAVELOAD_BYTE(stateMode, offset, data, splitScanline);
	SAVELOAD_WORD(stateMode, offset, data, splitAddr);

	SAVELOAD_BOOL(stateMode, offset, data, inFrame);
	SAVELOAD_BOOL(stateMode, offset, data, ppuIsReading);
	SAVELOAD_WORD(stateMode, offset, data, lastPPUReadAddr);
	SAVELOAD_BYTE(stateMode, offset, data, ppuReadAddrMatches);
	SAVELOAD_BYTE(stateMode, offset, data, ntReadCount);
	SAVELOAD_BYTE(stateMode, offset, data, ppuIdleCount);

	SAVELOAD_BYTE(stateMode, offset, data, multiplier1);
	SAVELOAD_BYTE(stateMode, offset, data, multiplier2);
	SAVELOAD_BOOL(stateMode, offset, data, timerRunning);
	SAVELOAD_BOOL(stateMode, offset, data, timerIRQ);
	SAVELOAD_WORD(stateMode, offset, data, timerCount);

	offset = MMC5sound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) {
		syncPRG();
		syncCHR();
		syncNT();
	}
	return offset;
}

int	MAPINT	mapperSound (int cycles) {
	int result = MMC5sound::Get(cycles);
	if (MMC5sound::HaveIRQ()) checkIRQs();
	return result;
}
} // namespace

