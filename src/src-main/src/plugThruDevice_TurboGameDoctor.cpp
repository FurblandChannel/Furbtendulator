#include "stdafx.h"

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "plugThruDevice.hpp"
#include "NES.h"
#include "FDS.h"
#include "Settings.h"

namespace PlugThruDevice {
namespace TurboGameDoctor {
union {
	uint8_t	k1 [8][1024];
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} biosTGD;
union {
	uint8_t	k1 [8][1024];
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} biosHGS;
union {
	uint8_t k8 [64][2][4096];
	uint8_t k32[16][8][4096];
	uint8_t	b  [512*1024];
} prgram;
union {
	uint8_t k1 [256][1024];
	uint8_t k8 [32][8][1024];
	uint8_t b  [256*1024];
} chrram;
uint8_t		scratch[2];
uint8_t		cache[64];

uint8_t		prgMask;
uint8_t		chrMask;
bool		haveHGS;

bool		gameMode;
bool		enableTGD;
bool		softGameSaver;
bool		enableFeedback;
bool		feedbackActive;
int		feedbackCount;
bool		interceptRequested;
bool		btIn0;
bool		btIn1;
bool		skip42FFCallbackClear;

bool		mapARAM;
bool		gameSaverActive;
uint8_t		bankWRAM;
uint8_t		bankHGS;

uint8_t		modeGD;
uint8_t		latchGD;
uint8_t		chrGD;
bool		protectPRG;
bool		lockCHR;
uint8_t		mirroring;
uint8_t		prgGD[4];
uint8_t		prgLoad;

bool		prgModeMC;
bool		prgModeTGD;
uint8_t		prgMC[4];

bool		chrModeTGD;
uint8_t		chrTGD[8];
uint8_t		lastCHRBank;

uint16_t	tgdCounter;
uint16_t	tgdTarget;

uint8_t		val43FF;

void		sync();

// Fastload callbacks
void	MAPINT	ppuWrite_gd (int bank, int addr, int val);
void	cbSkipGDHeader (void) {
	CPU::CPU[0]->PC =0xE54E;
}

void	cbReadFileHeader (void) {
	std::vector<uint8_t> header =FDS::diskReadBlock(3, 15);
	if (FDS::lastErrorCode ==0) for (int i =10; i <15; i++) FDS::putCPUByte(0x88 +i -10, header[i]);
	CPU::CPU[0]->IN_RTS();
}

void	cbReadFileCPU (void) {
	if (FDS::getCPUByte(0x80) &0x80) {
		FDS::putCPUWord(0xAC, 0x8000);
		FDS::putCPUWord(0x88, 0x8000);
	}
	std::vector<uint8_t> data =FDS::diskReadBlock(4, FDS::getCPUWord(0xAC));
	size_t bytesLeft =data.size();
	if (FDS::lastErrorCode ==0) {	
		int i =0;
		int addr =FDS::getCPUWord(0x0088);
		while (bytesLeft--) {
			FDS::putCPUByte(addr++, data[i++]);
			if (addr ==0xE000) addr =0x6000;
		}
		FDS::putCPUWord(0x0088, addr);
	}
	CPU::CPU[0]->IN_RTS();
}

void	cbReadFilePPU (void) {
	if (FDS::getCPUByte(0x80) &0x80) {
		int sizes[8] ={ 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x4000, 0x2000 };
		FDS::putCPUWord(0x88, 0x0000);
		FDS::putCPUWord(0xAC, sizes[FDS::getCPUByte(0x81) >>5]);
	}
	std::vector<uint8_t> data =FDS::diskReadBlock(4, FDS::getCPUWord(0xAC));
	size_t bytesLeft =data.size();
	if (FDS::lastErrorCode ==0) {	
		int i =0;
		int addr =FDS::getCPUWord(0x0088) &0x1FFF;
		while (bytesLeft--) {
			ppuWrite_gd(addr >>10, addr &0x3FF, data[i++]);
			if (++addr ==0x2000) {
				addr =0;
				uint8_t reg43FF =FDS::getCPUByte(0x90);
				reg43FF++;
				FDS::putCPUByte(0x0090, reg43FF);
				FDS::putCPUByte(0x43FF, reg43FF);
				if (~FDS::getCPUByte(0x80) &0x80 && chrMask ==0xFF) {
					reg43FF <<=3;
					reg43FF &=0xE0;
					FDS::putCPUByte(0x4400, reg43FF);
					FDS::putCPUByte(0x4401, reg43FF);
					FDS::putCPUByte(0x4402, reg43FF);
					FDS::putCPUByte(0x4403, reg43FF);
					FDS::putCPUByte(0x4404, reg43FF);
					FDS::putCPUByte(0x4405, reg43FF);
					FDS::putCPUByte(0x4406, reg43FF);
					FDS::putCPUByte(0x4407, reg43FF);
				}
			}
		}
		FDS::putCPUWord(0x0088, addr);
	}
	CPU::CPU[0]->IN_RTS();
}

void	cbReadFFETrainer (void) {
	std::vector<uint8_t> data =FDS::diskReadBlock(5, 512);
	if (FDS::lastErrorCode ==0) for (int i =0; i <512; i++) FDS::putCPUByte(0x5000 +i, data[i]);
	CPU::CPU[0]->PC =0xE539;
}

void	cbNextSide (void) {
	NES::eject28();
	NES::next28();
	NES::insert28();
	FDS::diskPosition =0;
	RI.changedDisk28 =false;
	
	std::vector<uint8_t> header =FDS::diskReadBlock(1, 55);
	if (FDS::lastErrorCode ==0) {
		std::vector<uint8_t> temp =FDS::diskReadBlock(2, 1);
		if (FDS::lastErrorCode ==0) FDS::putCPUByte(0x0006, temp[0]);
	}
	CPU::CPU[0]->PC =chrMask ==0x1F? 0xE699: 0xE69E;
}

// Always-enabled registers
int	MAPINT	cpuRead_reg (int bank, int addr) {
	if (addr >=0x5C0 && addr <=0x5FF)
		return cache[addr -0x5C0];
	else
	if (addr &0x800)
		if (haveHGS && gameSaverActive)
			return addr &0x400? biosHGS.b[addr |0x1000]: biosHGS.k1[bankHGS][addr &0x3FF];
		else
			return biosTGD.b[addr];
	else
	switch (addr) {
		case 0x400: case 0x401: case 0x402: case 0x403: case 0x404: case 0x405: case 0x406: case 0x407:
				return chrTGD[addr &7];
		case 0x408: case 0x409: case 0x40A: case 0x40B:
				return prgMC[addr &3] <<2 | latchGD &3;
		case 0x40C:	return tgdCounter >>8;
		case 0x40D:	return tgdCounter &0xFF;
		case 0x410:	return (gameMode?           0x01: 0x00) |
				       (enableTGD?          0x02: 0x00) |
				       (btIn0?              0x04: 0x00) |
				       (btIn1?              0x08: 0x00) |
				       (softGameSaver?      0x10: 0x00) |
				       (interceptRequested? 0x40: 0x00) |
				       (button?             0x80: 0x00);
		case 0x411:	return (bankWRAM                   ) |
				       (mapARAM?         0x04: 0x00) |
				       (gameSaverActive? 0x08: 0x00) |
				       (prgLoad &8?      0x20: 0x00) |
				       (chrModeTGD?      0x40: 0x00) |
				       (prgModeTGD?      0x80: 0x00);
		case 0x412: case 0x413:
				return scratch[addr &1];
		case 0x414:	return val43FF &0xF3;
		case 0x415:	return (modeGD <<5) | ((mirroring &1) <<4) | (protectPRG? 2: 0) | (mirroring &2? 1: 0);
		case 0x416:	return prgModeMC? 0x00: 0x01;
		case 0x417:	return 0x00;
		case 0x418:	return 0xCD;
		case 0x419:	return 0xEF;
		case 0x420:	return chrTGD[lastCHRBank];
		default:	return passCPURead(bank, addr);
	}
}
int	MAPINT	cpuReadDebug_reg (int bank, int addr) {
	if (addr >=0x5C0 && addr <=0x5FF)
		return cache[addr -0x5C0];
	else
	if (addr <0x400)
		return passCPUReadDebug(bank, addr);
	else
		return cpuRead_reg(bank, addr);
}
void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	if (addr >=0x5C0 && addr <=0x5FF)
		cache[addr -0x5C0] =val;
	else
	switch (addr) {
		case 0x181:	bankHGS =val &7;
				enableFeedback =!!(val &0x80);
				feedbackCount =1024;
				if (val ==0xFF) { // ???
					//gameSaverActive =false;
					enableTGD =true;
					gameMode =true;
					sync();
				}
				break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
				if (skip42FFCallbackClear)
					skip42FFCallbackClear =false;
				else
					CPU::callbacks.clear();
				enableTGD =true;
				gameMode =true;
				modeGD =val >>5;
				protectPRG =!!(addr &2);
				mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);
				if (modeGD >=4) lockCHR =false;
				sync();
				break;
		case 0x3FE: case 0x3FF:
				enableTGD =true;
				prgModeMC =!(addr &1);
				chrGD =val &3;
				latchGD =val;
				prgLoad =((val &0x30) >>4) |
				         ((val &0x08) >>1) |
					 (prgLoad &0x08);
				val43FF =val | (prgMask ==0x1F? 0x80: 0x00);
				sync();
				break;
		case 0x400: case 0x401: case 0x402: case 0x403: case 0x404: case 0x405: case 0x406: case 0x407:
				chrTGD[addr -0x400] =val;
				sync();
				break;
		case 0x40C:	pdSetIRQ(1);
				if (~val &0x80) tgdCounter =0x8000;
				tgdTarget =(tgdTarget &0x00FF) | (val <<8);
				break;
		case 0x40D:	pdSetIRQ(1);
				tgdTarget =(tgdTarget &0xFF00) | val;
				break;
		case 0x410:	gameMode =!!(val &0x01);
				enableTGD =!!(val &0x02);
				softGameSaver =!!(val &0x10);
				interceptRequested =!!(val &0x40);
				if (!interceptRequested) feedbackActive =false;
				sync();
				break;
		case 0x411:	bankWRAM =val &0x03;
				mapARAM =!!(val &0x04);
				gameSaverActive =!!(val &0x08);
				prgLoad =(prgLoad &~0x08) | (val &0x20? 0x08: 0x00);
				chrModeTGD =!!(val &0x40);
				prgModeTGD =!!(val &0x80);
				sync();
				break;
		case 0x412: case 0x413:
				scratch[addr &1] =val;
				break;
		case 0xFFF:	if (!enableTGD && Settings::FastLoad) {
					CPU::CPU[0]->PC =0xE53E;
					CPU::callbacks.clear();
					CPU::callbacks.push_back({0, 0xE540, cbSkipGDHeader});
					CPU::callbacks.push_back({0, 0xE050, cbReadFileHeader});
					CPU::callbacks.push_back({0, 0xE0A3, cbReadFileCPU});
					if (chrMask ==0x1F)
						CPU::callbacks.push_back({0, 0xE0F6, cbReadFilePPU});
					else
						CPU::callbacks.push_back({0, 0xE0FE, cbReadFilePPU});
					CPU::callbacks.push_back({0, 0xE656, cbNextSide});
					CPU::callbacks.push_back({0, 0xF45A, cbReadFFETrainer});
				}
				enableTGD =true;
				sync();
				break;
	}
}

bool	hvToggle;
int	MAPINT	cpuRead_ppuCache (int bank, int addr) {
	if ((addr &7) ==2) hvToggle =false;
	return passCPURead(bank, addr);
}
void	MAPINT	cpuWrite_ppuCache (int bank, int addr, int val) {
	switch (addr &7) {
		case 0:	cache[0] =val;
			break;
		case 1:	cache[1] =val;
			break;
		case 5:	if (hvToggle)
				cache[3] =val;
			else
				cache[2] =val;
			hvToggle =!hvToggle;
			break;
	}
	passCPUWrite(bank, addr, val);
}

// Loading state read/write handlers
static const uint8_t wram2pass[4] = { 0x8, 0xA, 0xC, 0x6 };
int	MAPINT	cpuRead_fdsram (int bank, int addr) {
	return passCPURead(wram2pass[gameMode? 3: bankWRAM] | (bank &1), addr);
}
void	MAPINT	cpuWrite_fdsram (int bank, int addr, int val) {
	passCPUWrite(wram2pass[gameMode? 3: bankWRAM] | (bank &1), addr, val);
}
int	MAPINT	nmiWatcher_gameSaverDisable (int bank, int addr) {
	if (addr ==0xFFA) {
		gameSaverActive =false;
		enableTGD =true;
		gameMode =true;
		sync();
		return (GetCPUReadHandler(bank))(bank, addr);
	} else
		return passCPURead(bank, addr);
}

int	MAPINT	cpuRead_load (int bank, int addr) {
	return prgram.k32[prgLoad &(prgMask >>2)][bank &7][addr];
}
void	MAPINT	cpuWrite_load (int bank, int addr, int val) {
	prgram.k32[prgLoad &(prgMask >>2)][bank &7][addr] =val;
}
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return biosTGD.k4[bank &1][addr];
}

// Playing state Game-Doctor-compatible mode read/write handlers
int	MAPINT	cpuRead_gd (int bank, int addr) {
	if ((softGameSaver || haveHGS) && interceptRequested && bank ==0xF && (addr &~1) ==0xFFA) {
		gameSaverActive =true;
		gameMode =false;
		enableTGD =false;
		return biosTGD.k4[bank &1][addr];
	}
	int index =(bank &7) >>1;
	return prgram.k8[prgGD[index] &prgMask][bank &1][addr];
}
void	MAPINT	cpuWrite_gd (int bank, int addr, int val) {
	if (protectPRG) {
		latchGD =val;
		prgMC[(bank &7) >>1] =val >>2;

		sync();
	} else {
		int index =(bank &7) >>1;
		prgram.k8[prgGD[index] &prgMask][bank &1][addr] =val;
	}
}

// Playing state Magic Card 8 KiB mode read/write handlers. CPU only; PPU is same as Game Doctor mode.
int	MAPINT	cpuRead_mc (int bank, int addr) {
	if ((softGameSaver || haveHGS) && interceptRequested && bank ==0xF && (addr &~1) ==0xFFA) {
		gameSaverActive =true;
		gameMode =false;
		enableTGD =false;
		return biosTGD.k4[bank &1][addr];
	}
	int index =(bank &7) >>1;
	int prgBank =prgMC[index] &0x0F;
	if (~bank &4) prgBank |=val43FF >>2 &0x10;
	return prgram.k8[prgBank &prgMask][bank &1][addr];
}
int	MAPINT	cpuRead_tgd (int bank, int addr) {
	if ((softGameSaver || haveHGS) && interceptRequested && bank ==0xF && (addr &~1) ==0xFFA) {
		gameSaverActive =true;
		gameMode =false;
		enableTGD =false;
		return biosTGD.k4[bank &1][addr];
	}
	int index =(bank &7) >>1;
	return prgram.k8[prgMC[index] &prgMask][bank &1][addr];
}
void	MAPINT	cpuWrite_mc (int bank, int addr, int val) {
	if (protectPRG)
		cpuWrite_gd (bank, addr, val);
	else {
		int index =(bank &7) >>1;
		int prgBank =prgMC[index] &0x0F;
		if (~bank &4) prgBank |=val43FF >>2 &0x10;
		prgram.k8[prgBank &prgMask][bank &1][addr] =val;
	}
}

// CHR read/write handlers
int	MAPINT	ppuRead_gd (int bank, int addr) {
	int chrOR =(chrTGD[bank] &0xE0) >>3;	
	return chrram.k8[(chrGD |chrOR) &chrMask][bank][addr];
}
void	MAPINT	ppuWrite_gd (int bank, int addr, int val) {
	int chrOR =(chrTGD[bank] &0xE0) >>3;
	if (modeGD >=5 && ~mirroring &2) lockCHR =!!(mirroring &1);
	if (modeGD <4 && !lockCHR ||
	    modeGD==4 && !lockCHR && ~mirroring &2 ||
	    !protectPRG ||
	    !gameMode) chrram.k8[(chrGD |chrOR) &chrMask][bank][addr] =val;
	// If TGD is disabled, note the CHR write in TGD's CHR-RAM, but pass-through nonetheless
	if (!enableTGD) passPPUWrite(bank, addr, val);
}
int	MAPINT	ppuRead_tgd (int bank, int addr) {
	return chrram.k1[chrTGD[bank] &chrMask][addr];
}
void	MAPINT	ppuWrite_tgd (int bank, int addr, int val) {
	if (modeGD >=5 && ~mirroring &2) lockCHR =!!(mirroring &1);
	if (modeGD <4 && !lockCHR ||
	    modeGD==4 && !lockCHR && ~mirroring &2 ||
	    !protectPRG ||
	    !gameMode) chrram.k1[chrTGD[bank] &chrMask][addr] =val;
}
int	MAPINT	ppuRead_bios (int bank, int addr) {
	return biosTGD.k1[4][addr];
}

// Nametable read/write handlers
int	MAPINT	readNT_V (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr];
}
int	MAPINT	readNT_H (int bank, int addr) {
	return PPU::PPU[0]->VRAM[(bank >>1) &1][addr];
}
int	MAPINT	readNT_0 (int bank, int addr) {
	return PPU::PPU[0]->VRAM[0][addr];
}
int	MAPINT	readNT_1 (int bank, int addr) {
	return PPU::PPU[0]->VRAM[1][addr];
}
void	MAPINT	writeNT_V (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		cache[addr &0x1F |0x20] =val;
	else
		PPU::PPU[0]->VRAM[bank &1][addr] =val;
}
void	MAPINT	writeNT_H (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		cache[addr &0x1F |0x20] =val;
	else
		PPU::PPU[0]->VRAM[(bank >>1) &1][addr] =val;
}
void	MAPINT	writeNT_0 (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		cache[addr &0x1F |0x20] =val;
	else
		PPU::PPU[0]->VRAM[0][addr] =val;
}
void	MAPINT	writeNT_1 (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		cache[addr &0x1F |0x20] =val;
	else
		PPU::PPU[0]->VRAM[1][addr] =val;
}

// Common
void	sync() {
	if (enableTGD) {
		if (gameMode) {
			// Convert the Game Doctor Latch register into a more usable form
			switch(modeGD) {
				case 0:	prgGD[0] =((latchGD &7) <<1) |0;
					prgGD[1] =((latchGD &7) <<1) |1;
					prgGD[2] =14;
					prgGD[3] =15;
					chrGD =0;
					break;
				case 1: prgGD[0] =((latchGD &~3) >>1) |0;
					prgGD[1] =((latchGD &~3) >>1) |1;
					prgGD[2] =14;
					prgGD[3] =15;
					chrGD =latchGD &3;
					break;
				case 2:	prgGD[0] =((latchGD &15) <<1) |0;
					prgGD[1] =((latchGD &15) <<1) |1;
					prgGD[2] =30;
					prgGD[3] =31;
					chrGD =0;
					break;
				case 3:	prgGD[0] =30;
					prgGD[1] =31;
					prgGD[2] =((latchGD &15) <<1) |0;
					prgGD[3] =((latchGD &15) <<1) |1;
					chrGD =latchGD >>4 &3;
					break;
				case 4:	prgGD[0] =((latchGD &0x30) >>2) |0;
					prgGD[1] =((latchGD &0x30) >>2) |1;
					prgGD[2] =((latchGD &0x30) >>2) |2;
					prgGD[3] =((latchGD &0x30) >>2) |3;
					chrGD =latchGD &3;
					break;
				case 5:	prgGD[0] =12;
					prgGD[1] =13;
					prgGD[2] =14;
					prgGD[3] =15;
					chrGD =latchGD &3;
					break;
				case 6:	prgGD[0] =latchGD &0xF;
					prgGD[1] =latchGD >>4;
					prgGD[2] =14;
					prgGD[3] =15;
					break;
				case 7:	prgGD[0] =latchGD &0xE;
					prgGD[1] =latchGD >>4 |1;
					prgGD[2] =14;
					prgGD[3] =15;
					chrGD =0;
					break;
			}
			if (prgModeMC) chrGD =latchGD &3;

			SetCPUReadHandler(0x5, passCPURead);
			SetCPUReadHandlerDebug(0x5, passCPUReadDebug);
			SetCPUWriteHandler(0x5, passCPUWrite);
			for (int bank =0x6; bank<=0x7; bank++) {
				SetCPUReadHandler     (bank, cpuRead_fdsram);
				SetCPUReadHandlerDebug(bank, cpuRead_fdsram);
				SetCPUWriteHandler    (bank, cpuWrite_fdsram);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				SetCPUReadHandler     (bank, prgModeTGD? cpuRead_tgd: prgModeMC? cpuRead_mc: cpuRead_gd);
				SetCPUReadHandlerDebug(bank, prgModeTGD? cpuRead_tgd: prgModeMC? cpuRead_mc: cpuRead_gd);
				SetCPUWriteHandler    (bank, (prgModeMC || prgModeTGD)? cpuWrite_mc: cpuWrite_gd);
			}
			for (int bank =0x0; bank <0x8; bank++) {
				SetPPUReadHandler     (bank, chrModeTGD? ppuRead_tgd: ppuRead_gd);
				SetPPUReadHandlerDebug(bank, chrModeTGD? ppuRead_tgd: ppuRead_gd);
				SetPPUWriteHandler    (bank, chrModeTGD? ppuWrite_tgd: ppuWrite_gd);
			}
		} else { // TGD enabled, but not playing game
			for (int bank =0x5; bank<=0x5; bank++) { // Always FDS RAM $7000 in $5xxx range
				SetCPUReadHandler     (bank, cpuRead_fdsram);
				SetCPUReadHandlerDebug(bank, cpuRead_fdsram);
				SetCPUWriteHandler    (bank, cpuWrite_fdsram);
			}
			for (int bank =0x6; bank<=0xD; bank++) {
				SetCPUReadHandler     (bank, mapARAM || gameSaverActive? cpuRead_fdsram:  cpuRead_load);
				SetCPUReadHandlerDebug(bank, mapARAM || gameSaverActive? cpuRead_fdsram:  cpuRead_load);
				SetCPUWriteHandler    (bank, mapARAM || gameSaverActive? cpuWrite_fdsram: cpuWrite_load);
			}
			for (int bank =0xE; bank<=0xF; bank++) {
				SetCPUReadHandler     (bank, cpuRead_bios);
				SetCPUReadHandlerDebug(bank, cpuRead_bios);
				SetCPUWriteHandler    (bank, passCPUWrite);
			}
			for (int bank =0x0; bank<=0x7; bank++) {
				SetPPUReadHandler     (bank, chrModeTGD? ppuRead_bios: ppuRead_gd);
				SetPPUReadHandlerDebug(bank, chrModeTGD? ppuRead_bios: ppuRead_gd);
				SetPPUWriteHandler    (bank, ppuWrite_gd);
			}
		}
		FPPURead readNT =NULL;
		FPPUWrite writeNT =NULL;
		switch(mirroring) {
			case 0: readNT =readNT_0; writeNT =writeNT_0; break;
			case 1: readNT =readNT_1; writeNT =writeNT_1; break;
			case 2: readNT =readNT_V; writeNT =writeNT_V; break;
			case 3: readNT =readNT_H; writeNT =writeNT_H; break;
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetPPUReadHandler     (bank, readNT);
			SetPPUReadHandlerDebug(bank, readNT);
			SetPPUWriteHandler    (bank, writeNT);
		}
	} else { // TGD disabled
		for (int bank =0x5; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, passCPURead);
			SetCPUReadHandlerDebug(bank, passCPUReadDebug);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		if (gameSaverActive) SetCPUReadHandler(0xF, nmiWatcher_gameSaverDisable);
		for (int bank =0x0; bank<=0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, ppuWrite_gd);
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, passPPUWrite);
		}
	}
	// PPU cache for Game Saver
	SetCPUReadHandler(0x2, cpuRead_ppuCache);
	SetCPUReadHandlerDebug(0x2, cpuReadDebug_reg);
	SetCPUWriteHandler(0x2, cpuWrite_ppuCache);
	
	// Registers in the $4xxx range are always enabled
	SetCPUReadHandler(0x4, cpuRead_reg);
	SetCPUReadHandlerDebug(0x4, cpuReadDebug_reg);
	SetCPUWriteHandler(0x4, cpuWrite_reg);

}

BOOL	MAPINT	load (void) {
	EnableMenuItem(hMenu, ID_GAME_BUTTON, MF_ENABLED);

	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}
BOOL	MAPINT	load4P (void) {
	loadBIOS (_T("BIOS\\VTGD4.BIN"), biosTGD.b, sizeof(biosTGD.b));
	if (Settings::HardGameSaver)
		haveHGS =loadBIOS (_T("BIOS\\VHGS.BIN"), biosHGS.b, sizeof(biosHGS.b));
	else
		haveHGS =false;
	Description =_T("Venus Turbo Game Doctor 4+");
	prgMask =0x3F;
	chrMask =0x1F;
	return load();
}
BOOL	MAPINT	load6P (void) {
	loadBIOS (_T("BIOS\\VTGD6.BIN"), biosTGD.b, sizeof(biosTGD.b));
	if (Settings::HardGameSaver)
		haveHGS =loadBIOS (_T("BIOS\\VHGS.BIN"), biosHGS.b, sizeof(biosHGS.b));
	else
		haveHGS =false;
	Description =_T("Venus Turbo Game Doctor 6+");
	prgMask =0x1F;
	chrMask =0xFF;
	return load();
}
BOOL	MAPINT	load6M (void) {
	loadBIOS (_T("BIOS\\VTGD6.BIN"), biosTGD.b, sizeof(biosTGD.b));
	if (Settings::HardGameSaver)
		haveHGS =loadBIOS (_T("BIOS\\VHGS.BIN"), biosHGS.b, sizeof(biosHGS.b));
	else
		haveHGS =false;
	Description =_T("Venus Turbo Game Doctor 6M");
	prgMask =0x3F;
	chrMask =0xFF;
	return load();
}


void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) {
		skip42FFCallbackClear =true;
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;		
		for (auto& c: cache) c =0x00;

		gameMode =false;
		enableTGD =true;
		softGameSaver =false;
		enableFeedback =false;
		feedbackActive =false;
		feedbackCount =0;
		interceptRequested =false;
		btIn0 =true;
		btIn1 =true;
		
		mapARAM =false;
		gameSaverActive =false;
		bankWRAM =3;
		bankHGS =0;

		modeGD =4;
		latchGD =0;
		chrGD =0;
		protectPRG =false;
		lockCHR =false;
		mirroring =0;
		prgLoad =0;
		for (int i =0; i <4; i++) prgGD[i] =i +12;

		prgModeMC =false;
		prgModeTGD =false;
		for (int i =0; i <4; i++) prgMC[i] =i +12;

		chrModeTGD =false;
		for (int i =0; i <8; i++) chrTGD[i] =i;
		lastCHRBank =0;

		tgdCounter =0;
		tgdTarget =0;
		val43FF =0;
	}
	for (auto& c: scratch) c =0x00; // Needs to be cleared on every reset
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (tgdTarget &0x8000) {
		if (tgdCounter ==tgdTarget && tgdCounter !=0xFFFF)
			pdSetIRQ(0);
		else
			tgdCounter++;
	}
	if (adMI->CPUCycle) adMI->CPUCycle();
	if (button) interceptRequested =true;
	if (enableFeedback && feedbackCount && !--feedbackCount) {
		feedbackActive =true;
		interceptRequested =true;
	}
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr <0x2000) lastCHRBank =addr >>10;
	if (adMI->PPUCycle) adMI->PPUCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	for (int i =0; i <=prgMask; i++) for (int j =0; j <8192; j++) SAVELOAD_BYTE(stateMode, offset, data, prgram.b[i *8192 +j]);
	for (int i =0; i <=chrMask; i++) for (int j =0; j <1024; j++) SAVELOAD_BYTE(stateMode, offset, data, chrram.b[i *1024 +j]);
	for (auto& c: scratch) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: cache) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BOOL(stateMode, offset, data, gameMode);
	SAVELOAD_BOOL(stateMode, offset, data, enableTGD);
	SAVELOAD_BOOL(stateMode, offset, data, softGameSaver);
	SAVELOAD_BOOL(stateMode, offset, data, enableFeedback);
	SAVELOAD_BOOL(stateMode, offset, data, feedbackActive);
	SAVELOAD_LONG(stateMode, offset, data, feedbackCount);
	SAVELOAD_BOOL(stateMode, offset, data, interceptRequested);
	SAVELOAD_BOOL(stateMode, offset, data, btIn0);
	SAVELOAD_BOOL(stateMode, offset, data, btIn1);

	SAVELOAD_BOOL(stateMode, offset, data, mapARAM);
	SAVELOAD_BOOL(stateMode, offset, data, gameSaverActive);
	SAVELOAD_BYTE(stateMode, offset, data, bankWRAM);
	SAVELOAD_BYTE(stateMode, offset, data, bankHGS);

	SAVELOAD_BYTE(stateMode, offset, data, modeGD);
	SAVELOAD_BYTE(stateMode, offset, data, latchGD);
	SAVELOAD_BYTE(stateMode, offset, data, chrGD);
	SAVELOAD_BOOL(stateMode, offset, data, protectPRG);
	SAVELOAD_BOOL(stateMode, offset, data, lockCHR);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, prgLoad);
	for (auto& c: prgGD) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BOOL(stateMode, offset, data, prgModeMC);
	SAVELOAD_BOOL(stateMode, offset, data, prgModeTGD);
	for (auto& c: prgMC) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BOOL(stateMode, offset, data, chrModeTGD);
	for (auto& c: chrTGD) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, lastCHRBank);

	SAVELOAD_WORD(stateMode, offset, data, tgdCounter);
	SAVELOAD_WORD(stateMode, offset, data, tgdTarget);
	SAVELOAD_BYTE(stateMode, offset, data, val43FF);

	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum4P =ID_PLUG_VENUS_TGD_4P;
uint16_t deviceNum6P =ID_PLUG_VENUS_TGD_6P;
uint16_t deviceNum6M =ID_PLUG_VENUS_TGD_6M;
} // namespace TurboGameDoctor

MapperInfo plugThruDevice_TurboGameDoctor4P ={
	&TurboGameDoctor::deviceNum4P,
	_T("Venus Turbo Game Doctor 4+"),
	COMPAT_FULL,
	TurboGameDoctor::load4P,
	TurboGameDoctor::reset,
	NULL,
	TurboGameDoctor::cpuCycle,
	TurboGameDoctor::ppuCycle,
	TurboGameDoctor::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_TurboGameDoctor6P ={
	&TurboGameDoctor::deviceNum6M,
	_T("Venus Turbo Game Doctor 6+"),
	COMPAT_FULL,
	TurboGameDoctor::load6P,
	TurboGameDoctor::reset,
	NULL,
	TurboGameDoctor::cpuCycle,
	TurboGameDoctor::ppuCycle,
	TurboGameDoctor::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_TurboGameDoctor6M ={
	&TurboGameDoctor::deviceNum6M,
	_T("Venus Turbo Game Doctor 6M"),
	COMPAT_FULL,
	TurboGameDoctor::load6M,
	TurboGameDoctor::reset,
	NULL,
	TurboGameDoctor::cpuCycle,
	TurboGameDoctor::ppuCycle,
	TurboGameDoctor::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice