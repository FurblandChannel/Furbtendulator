#include "stdafx.h"
#include <vector>
#include <queue>
#include <mutex>

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "simplefdc.hpp"
#include "plugThruDevice.hpp"
#include "plugThruDevice_SuperMagicCard_transfer.hpp"
#include "Settings.h"

namespace PlugThruDevice {
namespace SuperMagicCard {
std::queue<uint8_t> receiveQueue;
std::queue<uint8_t> sendQueue;
std::queue<uint8_t> externalQueue;
std::mutex	receiveQueueBusy;
std::mutex	sendQueueBusy;
std::mutex	externalQueueBusy;
FDC		fdc;

bool		passThrough;
bool		fdcEnable;
bool		mapBIOS;
uint8_t		wramBank;

union {
	uint8_t	k4 [4][4096];
	uint8_t	b  [16*1024];
} bios;
union {
	uint8_t k8 [64][2][4096];
	uint8_t k16[32][4][4096];
	uint8_t	b  [512*1024];
} prgram;
union {
	uint8_t k1 [256][1024];
	uint8_t k8 [32][8][1024];
	uint8_t b  [256*1024];
} chrram;
union {
	uint8_t	k8 [4][2][4096];
	uint8_t	b  [32*1024];
} wram;
uint8_t		ram[4*1024];

uint8_t		modeGD;
uint8_t		latchGD;
uint8_t		chrGD;
bool		protectPRG;
bool		lockCHR;
uint8_t		mirroring;
uint8_t		page8;
uint8_t		pageC;

bool		prgModeMC;
bool		prgModeSMC;
bool		chrModeSMC;
bool		chrModeMMC4;
bool		ciramEnable;
uint8_t		prgSMC[4];
uint8_t		chrSMC[12];
uint8_t		mmc4State[2];

bool		smcUsePA12;
bool		smcIRQ;
uint16_t	smcPrevPA;
int16_t		smcCounter;

void		sync();

int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios.k4[bank &3][addr];
}
int	MAPINT	cpuRead_ram (int bank, int addr) {
	return ram[addr];
}
void	MAPINT	cpuWrite_ram (int bank, int addr, int val) {
	ram[addr] =val;
}
int	MAPINT	cpuRead_wram (int bank, int addr) {
	return wram.k8[wramBank][bank &1][addr];
}
void	MAPINT	cpuWrite_wram (int bank, int addr, int val) {
	wram.k8[wramBank][bank &1][addr] =val;
}

int	MAPINT	cpuRead_reg (int bank, int addr) {
	std::lock_guard<std::mutex> lock(receiveQueueBusy);

	switch (addr) {
		case 0x500:	return (receiveQueue.empty()? 0x00: 0x80) | (button? 0x00: 0x40) | (modeGD <<3) | (mirroring &1? 4: 0) | (protectPRG? 2: 0) | (mirroring &2? 1: 0);
		case 0x501:	return (prgModeSMC? 0: 2) | (prgModeMC? 0: 1) | (latchGD <<2);
		case 0x508: case 0x509: case 0x50A: case 0x50B: case 0x50C: case 0x50D: case 0x50E: case 0x50F:
				if (fdcEnable)
					return fdc.readIO(addr);
				else {
					int result =receiveQueue.empty()? 0x00: receiveQueue.front() &0xFF;
					receiveQueue.pop();
					if ((receiveQueue.size() &0xFFF) ==0)  {
						if (receiveQueue.empty())
							EI.StatusOut(L"");
						else
							EI.StatusOut(L"SMC is receiving data (%u KiB left)", receiveQueue.size() >>10);
					}
					return result;
				}
		default:	return passCPURead(bank, addr);
	}
}

int	MAPINT	cpuReadDebug_reg (int bank, int addr) {
	std::lock_guard<std::mutex> lock(receiveQueueBusy);

	switch (addr) {
		case 0x500:	return (receiveQueue.empty()? 0x00: 0x80) | (button? 0x00: 0x40) | (modeGD <<3) | (mirroring &2? 1: 0) | (protectPRG? 2: 0);
		case 0x501:	return (prgModeSMC? 0: 2) | (prgModeMC? 0: 1) | (latchGD <<2);
		case 0x508: case 0x509: case 0x50A: case 0x50B: case 0x50C: case 0x50D: case 0x50E: case 0x50F:
				if (fdcEnable)
					return fdc.readIODebug(addr);
				else
					return receiveQueue.empty()? 0x00: receiveQueue.front() &0xFF;
		default:	return passCPURead(bank, addr);
	}
}

void	MAPINT	cpuWrite_cache (int bank, int addr, int val) {
	if (addr ==0x025 && smcIRQ) return;
	passCPUWrite(bank, addr, val);
	if (bank ==0x2) ram[addr &0x07 |0x800] =val;	                // Cache PPU registers for real-time save functionality
	if (bank ==0x4 && addr >=0x000 && addr <=0x01F) ram[addr] =val; // Cache Sound registers for real-time save functionality
	if (bank ==0x4 && addr >=0x500 && addr <=0x51F) ram[addr] =val; // Cache Super MagicCard registers for real-time save functionality
}

void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	cpuWrite_cache(bank, addr, val);
	switch (addr) {
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
				protectPRG =!!(addr &2);
				modeGD =val >>5;
				mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);
				if (modeGD >=4) lockCHR =false;
				sync();
				break;
		case 0x3FC: case 0x3FD: case 0x3FE: case 0x3FF:
				prgModeMC  =!(addr &1);
				prgModeSMC =!(addr &2) && !(addr &1);
				if (passThrough) {
					passThrough =false;
					mapBIOS =true;
				}
				latchGD =val;
				sync();
				break;
		case 0x500:	chrModeSMC  =!!(val &0x01);
				ciramEnable =!!(val &0x02);
				chrModeMMC4 =chrModeSMC && !(val &0x04);
				smcUsePA12  =!!(val &0x08);
				fdcEnable   =!!(val &0x08);
				wramBank    =  (val &0x30) >>4;
				mapBIOS     = !(val &0x40);
				passThrough =!!(val &0x80);
				
				if (fdcEnable) fdc.reset();
				sync();
				break;
		case 0x501:	smcIRQ =false;
				adSetIRQ(1);
				pdSetIRQ(1);
				break;
		case 0x502:	smcCounter =(smcCounter &0xFF00) |val;
				adSetIRQ(1);
				pdSetIRQ(1);
				break;
		case 0x503:	smcCounter =(smcCounter &0x00FF) |(val <<8);
				smcIRQ =true;
				adSetIRQ(1);
				pdSetIRQ(1);
				break;
		case 0x504: case 0x505: case 0x506: case 0x507:
				if (!prgModeSMC) val >>=2;
				prgSMC[addr -0x504] =val;
				sync();
				break;
		case 0x508: case 0x509: case 0x50A: case 0x50B: case 0x50C: case 0x50D: case 0x50E: case 0x50F:
				if (fdcEnable)
					fdc.writeIO(addr, val);
				else {
					std::lock_guard<std::mutex> lock(sendQueueBusy);
					sendQueue.push(val);
				}
				break;
		case 0x510: case 0x511: case 0x512: case 0x513: case 0x514: case 0x515: case 0x516: case 0x517: case 0x518: case 0x519: case 0x51A: case 0x51B:
				chrSMC[addr -0x510] =val;
				sync();
				break;
	}
}

// Game-Doctor-compatible mode read/write handlers
int	MAPINT	ppuRead_gd (int bank, int addr) {
	return chrram.k8[chrGD][bank][addr];
}
void	MAPINT	ppuWrite_gd (int bank, int addr, int val) {
	if (modeGD >=5 && ~mirroring &2) lockCHR =!!(mirroring &1);
	if (modeGD <4 && !lockCHR ||
	    modeGD==4 && !lockCHR && ~mirroring &2 ||
	    !protectPRG) chrram.k8[chrGD][bank][addr] =val;
}
int	MAPINT	cpuRead_gd (int bank, int addr) {
	if (bank <0xC)
		return prgram.k16[page8][bank &3][addr];
	else
		return prgram.k16[pageC][bank &3][addr];
}
void	MAPINT	cpuWrite_gd (int bank, int addr, int val) {
	if (protectPRG) {
		latchGD =val;
		if (!prgModeSMC) prgSMC[(bank &7) >>1] =val >>2 &0x3F;
		
		sync();
	} else
	if (bank <0xC)
		prgram.k16[page8][bank &3][addr] =val;
	else
		prgram.k16[pageC][bank &3][addr] =val;
}

// MagicCard 8 KiB mode read/write handlers. CPU only; PPU is same as Game Doctor mode.
int	MAPINT	cpuRead_mc (int bank, int addr) {
	int index =(bank &7) >>1;
	return prgram.k8[prgSMC[index] &0x1F][bank &1][addr];
}
void	MAPINT	cpuWrite_mc (int bank, int addr, int val) {
	if (protectPRG)
		cpuWrite_gd (bank, addr, val);
	else {
		int index =(bank &7) >>1;
		prgram.k8[prgSMC[index] &0x1F][bank &1][addr] =val;
	}
}

// Super Magic Card mode read/write handlers
int	MAPINT	ppuRead_mmc4 (int bank, int addr) {
	int reg =(bank &5) | (mmc4State[bank &4? 1: 0]);
	if (bank ==3) {
		if ((addr &0x3F8) ==0x3D8) mmc4State[0] =0; else
		if ((addr &0x3F8) ==0x3E8) mmc4State[0] =2;
	} else
	if (bank ==7) {
		if ((addr &0x3F8) ==0x3D8) mmc4State[1] =0; else
		if ((addr &0x3F8) ==0x3E8) mmc4State[1] =2;
	}
	return chrram.k1[chrSMC[reg] &~3 | bank &3][addr];
}
int	MAPINT	ppuRead_smc (int bank, int addr) {
	return chrram.k1[chrSMC[bank]][addr];
}
void	MAPINT	ppuWrite_smc (int bank, int addr, int val) {
	if (modeGD >=5 && ~mirroring &2) lockCHR =!!(mirroring &1);
	if (modeGD <4 && !lockCHR ||
	    modeGD==4 && !lockCHR && ~mirroring &2 ||
	    !protectPRG) chrram.k1[chrSMC[bank]][addr] =val;
}
int	MAPINT	cpuRead_smc (int bank, int addr) {
	int index =(bank &7) >>1;
	return prgram.k8[prgSMC[index] &0x3F][bank &1][addr];
}
void	MAPINT	cpuWrite_smc (int bank, int addr, int val) {
	if (protectPRG)
		cpuWrite_gd (bank, addr, val);
	else {
		int index =(bank &7) >>1;
		prgram.k8[prgSMC[index] &0x3F][bank &1][addr] =val;
	}
}

// Nametable and palette cache handler
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
int	MAPINT	readNT_4 (int bank, int addr) {
	return chrram.k1[chrSMC[bank &3 |8]][addr];
}
void	MAPINT	writeNT_V (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		ram[addr &0x1F |0x820] =val;
	else
		PPU::PPU[0]->VRAM[bank &1][addr] =val;
}
void	MAPINT	writeNT_H (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		ram[addr &0x1F |0x820] =val;
	else
		PPU::PPU[0]->VRAM[(bank >>1) &1][addr] =val;
}
void	MAPINT	writeNT_0 (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		ram[addr &0x1F |0x820] =val;
	else
		PPU::PPU[0]->VRAM[0][addr] =val;
}
void	MAPINT	writeNT_1 (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		ram[addr &0x1F |0x820] =val;
	else
		PPU::PPU[0]->VRAM[1][addr] =val;
}
void	MAPINT	writeNT_4 (int bank, int addr, int val) {
	if (bank ==0xF && addr >=0xF00) // Cache palette
		ram[addr &0x1F |0x820] =val;
	else
		chrram.k1[chrSMC[bank &3 |8]][addr] =val;
}

// Common
void	sync() {
	// Convert the Game Doctor Latch register into a more usable form
	switch(modeGD) {
		case 0:	page8 =latchGD &7;
			pageC =7;
			chrGD =0;
			break;
		case 1: page8 =(latchGD >>2) &15;
			pageC =7;
			chrGD =latchGD &3;
			break;
		case 2:	page8 =latchGD &15;
			pageC =15;
			chrGD =0;
			break;
		case 3:	page8 =15;
			pageC =latchGD &15;
			chrGD =latchGD >>4 &3;
			break;
		case 4:	page8 =(latchGD >>3) &0x06;
			pageC =(latchGD >>3) &0x06 |1;
			chrGD =latchGD &3;
			break;
		case 5:	page8 =6;
			pageC =7;
			chrGD =latchGD &3;
			break;
		case 6:	page8 =6;
			pageC =7;
			chrGD =latchGD &1;
			break;
		case 7:	page8 =6;
			pageC =7;
			chrGD =0;
			break;
	}
	if (prgModeMC || prgModeSMC) chrGD =latchGD &3;
	
	if (passThrough) {	// Pass through whatever is in the cartridge slot in the $6000-$FFFF area. $4xxx writeable but not readable, $5xxx readable but not writeable.
		for (int bank =0x0; bank <0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, passPPUWrite);
		}		
		for (int bank =0x6; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, passCPURead);
			SetCPUReadHandlerDebug(bank, passCPUReadDebug);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		SetCPUWriteHandler(0x2, cpuWrite_cache);

		SetCPUReadHandler(0x4, passCPURead);
		SetCPUReadHandlerDebug(0x4, passCPUReadDebug);
		SetCPUWriteHandler(0x4, cpuWrite_reg);

		SetCPUReadHandler(0x5, cpuRead_ram);
		SetCPUReadHandlerDebug(0x5, cpuRead_ram);
		SetCPUWriteHandler(0x5, passCPUWrite);
	} else {		// SMC GUI or Play mode
		SetCPUWriteHandler(0x2, cpuWrite_cache);
		
		SetCPUReadHandler(0x4, cpuRead_reg);
		SetCPUReadHandlerDebug(0x4, cpuReadDebug_reg);
		SetCPUWriteHandler(0x4, cpuWrite_reg);
		
		SetCPUReadHandler(0x5, cpuRead_ram);
		SetCPUReadHandlerDebug(0x5, cpuRead_ram);
		SetCPUWriteHandler(0x5, cpuWrite_ram);
		
		if (mapBIOS) {	// Memory map while GUI is shown: SMC BIOS at $C000-$FFFF, PRG/WRAM at $6000-$BFFF
			for (int bank =0x6; bank<=0xB; bank++) {
				if (protectPRG && bank <0x8)  {
					SetCPUReadHandler(bank, cpuRead_wram);
					SetCPUReadHandlerDebug(bank, cpuRead_wram);
					SetCPUWriteHandler(bank, cpuWrite_wram);
				} else {
					SetCPUReadHandler(bank, cpuRead_smc);
					SetCPUReadHandlerDebug(bank, cpuRead_smc);
					SetCPUWriteHandler(bank, cpuWrite_smc);
				}
			}
			for (int bank =0xC; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, cpuRead_bios);
				SetCPUReadHandlerDebug(bank, cpuRead_bios);
				SetCPUWriteHandler(bank, passCPUWrite);
			}
		} else {	// Memory while playing loaded game: WRAM at $6000-$7FFF, PRG at $8000-$FFFF
			for (int bank =0x6; bank<=0x7; bank++) {
				SetCPUReadHandler(bank, cpuRead_wram);
				SetCPUReadHandlerDebug(bank, cpuRead_wram);
				SetCPUWriteHandler(bank, cpuWrite_wram);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				if (prgModeSMC) {
					SetCPUReadHandler(bank, cpuRead_smc);
					SetCPUReadHandlerDebug(bank, cpuRead_smc);
					SetCPUWriteHandler(bank, cpuWrite_smc);
				} else
				if (prgModeMC) {
					SetCPUReadHandler(bank, cpuRead_mc);
					SetCPUReadHandlerDebug(bank, cpuRead_mc);
					SetCPUWriteHandler(bank, cpuWrite_mc);
				} else {
					SetCPUReadHandler(bank, cpuRead_gd);
					SetCPUReadHandlerDebug(bank, cpuRead_gd);
					SetCPUWriteHandler(bank, cpuWrite_gd);
				}
			}
		}
		// CHR banking is independent of CPU memory map
		for (int bank =0x0; bank <0x8; bank++) {
			if (chrModeSMC) {
				SetPPUReadHandler(bank, chrModeMMC4? ppuRead_mmc4: ppuRead_smc);
				SetPPUReadHandlerDebug(bank, ppuRead_smc);
				SetPPUWriteHandler(bank, ppuWrite_smc);
			} else {
				SetPPUReadHandler(bank, ppuRead_gd);
				SetPPUReadHandlerDebug(bank, ppuRead_gd);
				SetPPUWriteHandler(bank, ppuWrite_gd);
			}
		}
		// NT banking is independent of CPU memory map
		FPPURead readNT =readNT_4;
		FPPUWrite writeNT =writeNT_4;
		if (ciramEnable) switch(mirroring) {
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
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS(_T("BIOS\\FFESMC.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Front Fareast Super Magic Card");
	EnableMenuItem(hMenu, ID_GAME_BUTTON, MF_ENABLED);

	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) {
		receiveQueue =std::queue<uint8_t>();
		sendQueue =std::queue<uint8_t>();
		fdc.reset();
		
		passThrough =false;
		fdcEnable =false;
		mapBIOS =true;
		wramBank =0;
		
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;
		for (auto& c: wram.b) c =0x00;
		for (auto& c: ram) c =0x00;
		
		modeGD =0;
		latchGD =0;
		chrGD =0;
		protectPRG =true;
		lockCHR =false;
		mirroring =2;
		page8 =0;
		pageC =1;
		
		prgModeMC =false;	
		prgModeSMC =false;
		chrModeSMC =false;
		chrModeMMC4 =false;
		ciramEnable =false;
		for (auto c: prgSMC) c =0;
		for (auto c: chrSMC) c =0;
		for (auto c: mmc4State) c =0;
		
		smcUsePA12 =false;
		smcIRQ =false;
		smcPrevPA =0;
		smcCounter =0;
		
		hardReset =true;
	}
	CPU::callbacks.clear();
	sync();
}

void	clockSMCCounter (void) {
	if (smcIRQ) {
		if (smcCounter ==-1) {
			smcIRQ =false;
			pdSetIRQ(0);
		} else
			smcCounter++;
	}
}

void	MAPINT	cpuCycle (void) {
	if (RI.changedDisk35) {
		if (RI.diskData35)
			fdc.insertDisk(1, &(*RI.diskData35)[0], RI.diskData35->size(), false);
		else
			fdc.ejectDisk(1);
		RI.changedDisk35 =false;
	}
	if (!smcUsePA12) clockSMCCounter();
	if (adMI->CPUCycle) adMI->CPUCycle();
	
	fdc.run();
	doAnyTransfer();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (smcUsePA12 && addr &0x1000 && ~smcPrevPA &0x1000) clockSMCCounter();
	smcPrevPA =addr;
	if (adMI->PPUCycle) adMI->PPUCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, passThrough);
	SAVELOAD_BOOL(stateMode, offset, data, fdcEnable);
	SAVELOAD_BOOL(stateMode, offset, data, mapBIOS);
	SAVELOAD_BYTE(stateMode, offset, data, wramBank);

	for (auto& c: prgram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: wram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: ram) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	SAVELOAD_BYTE(stateMode, offset, data, modeGD);
	SAVELOAD_BYTE(stateMode, offset, data, latchGD);
	SAVELOAD_BYTE(stateMode, offset, data, chrGD);
	SAVELOAD_BOOL(stateMode, offset, data, protectPRG);
	SAVELOAD_BOOL(stateMode, offset, data, lockCHR);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, page8);
	SAVELOAD_BYTE(stateMode, offset, data, pageC);

	SAVELOAD_BOOL(stateMode, offset, data, prgModeMC);
	SAVELOAD_BOOL(stateMode, offset, data, prgModeSMC);
	SAVELOAD_BOOL(stateMode, offset, data, chrModeSMC);
	SAVELOAD_BOOL(stateMode, offset, data, chrModeMMC4);
	SAVELOAD_BOOL(stateMode, offset, data, ciramEnable);
	for (auto& c: prgSMC) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chrSMC) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: mmc4State) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	SAVELOAD_BOOL(stateMode, offset, data, smcUsePA12);
	SAVELOAD_BOOL(stateMode, offset, data, smcIRQ);
	SAVELOAD_WORD(stateMode, offset, data, smcPrevPA);
	SAVELOAD_WORD(stateMode, offset, data, smcCounter);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_FFE_SMC;
} // namespace SuperMagicCard

MapperInfo plugThruDevice_SuperMagicCard ={
	&SuperMagicCard::deviceNum,
	_T("Front Fareast Super Magic Card"),
	COMPAT_FULL,
	SuperMagicCard::load,
	SuperMagicCard::reset,
	NULL,
	SuperMagicCard::cpuCycle,
	SuperMagicCard::ppuCycle,
	SuperMagicCard::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice

/*
Still left to do:
	1.	Transferring .RTS files via the parallel interface is not implemented yet.
*/