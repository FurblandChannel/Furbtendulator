#include "stdafx.h"

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "PPU.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_GameMaster.hpp"

#include "NES.h"

namespace PlugThruDevice {
namespace GameMaster {
bool		enableMasterLink;
bool		v20;
uint32_t	prgramSize;
uint32_t	chrramSize;
uint8_t 	ramMask;

BIOS_GM  	bios;
PRGRAM   	prgram;
CHRRAM   	chrram;
StateRAM	stateRAM;
uint8_t		workRAM[2048];

bool		initialized;
bool		map6toD;
bool		mapEtoF;
bool		mapBIOS;
bool		mapMode;
bool		mapAuxiliary;
bool		mapWorkRAM;
bool		mapSGD;
bool		mapCartridge;
bool		ciramCHR;

uint8_t		pageRead;
uint8_t		pageWrite;

uint8_t		parallelDataGM;
uint8_t		parallelStateGM;
uint8_t		transferByte;
bool		frameTimerEnabled;
uint8_t		frameTimerCount;
bool		hvToggle;
uint8_t		cache[0x50];
uint8_t		controllerCount;
uint8_t		controllerData;

bool		intercept;
bool		selectKey;
uint8_t		inputState;

bool		logEnabled;
uint16_t	logHead;
uint16_t	logTail;
uint16_t	logMask;

void		sync();

// CPU write logger for mapper register cache
const uint8_t	cacheVal2Addr [16] = { 0, 1, 2, 3, 11, 12, 13, 14, 16, 16, 16, 16, 16, 16, 16, 16 };
uint8_t		cacheCount;
uint8_t		cacheShift;

void	MAPINT	cpuWrite_log(int bank, int addr, int val) {
	//if (map6toD) return;
	if (bank >=0x6 && bank <=0xD && map6toD ||
	    bank >=0xE && bank <=0xF && mapEtoF ||
	    bank ==0x5 && mapWorkRAM            ||
	    bank <=0x4)
	    ;
	else {
		uint8_t a0 =cacheVal2Addr[cache[0x08] &0xF];
		uint8_t a1 =cacheVal2Addr[cache[0x09] &0xF];
		uint8_t a2 =cacheVal2Addr[cache[0x0A] &0xF];
		uint8_t a3 =cacheVal2Addr[cache[0x0B] &0xF];
		uint16_t fullAddr =(bank <<12) |addr;
		uint8_t cacheAddr =(fullAddr &(1 <<a0)? 0x01: 0x00) |
				(fullAddr &(1 <<a1)? 0x02: 0x00) |
				(fullAddr &(1 <<a2)? 0x04: 0x00) |
				(fullAddr &(1 <<a3)? 0x08: 0x00) |0x10;
		if (bank >=(cache[0x0C] &0x80? 0x6: 0x8)) {
			if (cache[0x0C] &0x04) {	// MMC1-like shift register
				if (val &0x80)
					cacheCount =cacheShift =0;
				else {
					cacheShift =(cacheShift <<1) |(val &1);
					if (++cacheCount ==5) {
						cache[cacheAddr] =cacheShift;
						cacheCount =cacheShift =0;						
					}
				}
			} else {
				cache[cacheAddr] =val;
				if (cache[0xC] &0x40 && cacheAddr ==0x11) cache[0x18 +(cache[0x10] &0x7)] =cache[0x11]; // MMC3 indexed register
			}
		}
		if (logEnabled) {
			uint16_t maskedAddr =fullAddr &(logMask |0x8000);
			int currentAddress;
			bool found =false;
			for (currentAddress =0; currentAddress <logTail; currentAddress +=3) {
				uint16_t checkAddr =((stateRAM.k4[4][currentAddress +0] <<8) | stateRAM.k4[4][currentAddress +1]) &(logMask |0x8000);
				if (checkAddr ==maskedAddr) {
					found =true;
					break;
				}
			}
			if (found) {
				stateRAM.k4[4][currentAddress +0] =fullAddr >>8;
				stateRAM.k4[4][currentAddress +1] =fullAddr &0xFF;
				stateRAM.k4[4][currentAddress +2] =val;
			} else {
				stateRAM.k4[4][logTail +0] =fullAddr >>8;
				stateRAM.k4[4][logTail +1] =fullAddr &0xFF;
				stateRAM.k4[4][logTail +2] =val;
				logTail +=3;
			}
			if (logTail >=0xFF0) logTail -=0xFF0;
			logHead =0;
		} // restored at $0300
	}
	if (mapSGD)
		cpuWrite_sgd(bank, addr, val);
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	cpuRead_ram (int bank, int addr) {
	if (mapWorkRAM || addr !=0x204 && addr !=0x205 && addr <0x800)
		return workRAM[addr &0x7FF];
	else
		return passCPURead(bank, addr);
}
void	MAPINT	cpuWrite_ram (int bank, int addr, int val) {
	workRAM[addr] =val;
}

// Handlers for PRG-RAM in the $6000-$DFFF range. Page F goes to State RAM instead.
// If map6toD is false, then passread/passWrite handler will be set instead.
int	MAPINT	cpuRead_prgram (int bank, int addr) {
	if (pageRead ==0xF)
		return stateRAM.k4[bank &7][addr];
	else
		return prgram.k32[pageRead &ramMask][bank &7][addr];
}
void	MAPINT	cpuWrite_prgram (int bank, int addr, int val) {
	if (pageWrite ==0xF && map6toD)
		stateRAM.k4[bank &7][addr] =val;
	else
		prgram.k32[pageWrite &ramMask][bank &7][addr] =val;
}

// Special handler for the $E000-$FFFF range. Page F goes to State RAM instead.
int	MAPINT	cpuRead_EtoF (int bank, int addr) {
	if (mapEtoF && (!mapSGD || !mapAuxiliary)) {
		if (mapBIOS)
			return bios.k8[mapMode && mapSGD? 0: mapMode? 3: 2][bank &1][addr];
		else
		if (mapMode)
			return stateRAM.k4[bank &7][addr];
		else
		if (mapSGD)
			return cpuRead_sgd(bank, addr);
		else
			return passCPURead(bank, addr);
	} else {
		if (bank ==0xF && addr ==0xFFA) {
			inputState &=~0x20;
			if (frameTimerEnabled) {
				frameTimerCount--;
				if (frameTimerCount ==0xFF) {
					intercept =true;
					inputState |=0x20;
					frameTimerEnabled =false;
				}
			}
			if (intercept) {
				intercept =false;
				mapEtoF =true;
				mapBIOS =true;
				mapMode =true;
				mapSGD =false;
				sync();
				return (GetCPUReadHandler(bank))(bank, addr);
			}
		}
		if (mapMode && !mapSGD)
			return stateRAM.k4[bank &7][addr];	// Soft-patched   FDS BIOS
		else
			return mapSGD? cpuRead_sgd(bank, addr): passCPURead(bank, addr);
	}
}
int	MAPINT	cpuReadDebug_EtoF (int bank, int addr) {
	if (mapEtoF && (!mapSGD || !mapAuxiliary)) {
		if (mapBIOS)
			return bios.k8[mapMode && mapSGD? 0: mapMode? 3: 2][bank &1][addr];
		else
		if (mapMode)
			return stateRAM.k4[bank &7][addr];
		else
		if (mapSGD)
			return cpuReadDebug_sgd(bank, addr);
		else
			return passCPUReadDebug(bank, addr);
	} else
		if (mapMode && !mapSGD)
			return stateRAM.k4[bank &7][addr];
		else
			return mapSGD? cpuReadDebug_sgd(bank, addr): passCPUReadDebug(bank, addr);
}

// General registers
int	MAPINT	cpuRead_reg (int bank, int addr) {
	if (addr ==0x016) {
		int result =cpuRead_sgd(bank, addr);
		controllerData =(controllerData <<1) | (result &1);
		controllerCount++;
		inputState &=~0x07;
		if (controllerCount ==8 && (controllerData &0x20 && selectKey || controllerData &0x10 && !selectKey)) {
			switch (controllerData >>6) {
				case 1:	inputState |=0x02; intercept =true; break; // restore
				case 2:	inputState |=0x01; intercept =true; break; // save
				case 3:	inputState |=0x04; intercept =true; break; // menu
			}
		}
		return result;
	} else
	if (addr >=0x190 && addr <=0x1DF)
		return cache[addr -0x190];
	else
	switch (addr) {
		case 0x048:	return 0x80;
		case 0x180:	return (mapMode?      0x01: 0x00) |
				       (mapBIOS?      0x02: 0x00) |
				       (mapAuxiliary? 0x10: 0x00) |
				       (ciramCHR?     0x40: 0x00) |
				       (map6toD?      0x80: 0x00);
		case 0x181:	return pageRead | (pageWrite <<4);
		case 0x182:	return frameTimerCount;
		case 0x183:	return transferByte;
		case 0x184:	return (parallelStateML ==0xCE? 0x00: 0x08) | (parallelStateML ==0xCB? 0x00: 0x02);
		case 0x185:	return (initialized?  0x01: 0x00) |
				       (selectKey?    0x02: 0x00) |
				       (mapCartridge? 0x04: 0x00) |
				       (logEnabled?   0x08: 0x00) |
				       (mapSGD?       0x10: 0x00) |
				       (mapWorkRAM?   0x20: 0x00) |
				       (mapEtoF?      0x80: 0x00);
		case 0x186:	return logMask >>8;
		case 0x187:	return logMask &0xFF;
		case 0x188:	return logTail &0xFF;	// address at which restoration ends
		case 0x189:	return logTail >>8;
		case 0x18A:	return logHead &0xFF;	// address at which STA $4189/$418B occurs during restoration
		case 0x18B:	return logHead >>8;
		case 0x18D:	return inputState |0x80;
		default:	return cpuRead_sgd(bank, addr);
	}
}
int	MAPINT	cpuReadDebug_reg (int bank, int addr) {
	if (addr >=0x180 && addr <=0x1DF)
		return cpuRead_reg(bank, addr);
	else
		return cpuReadDebug_sgd(bank, addr);
}
void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	if (addr >=0x000 && addr <=0x015) // Cache APU write
		cache[addr -0x000 +0x020] =val;
	else
	if (addr ==0x016 && val &1)
		controllerCount =0;

	if (addr >=0x190 && addr <=0x1DF) // Cache direct write
		cache[addr -0x190] =val;
	else
	switch (addr) {
		case 0x180:	mapMode   =!!(val &0x01);
				mapBIOS   =!!(val &0x02);
				mapAuxiliary =!!(val &0x10);
				ciramCHR  =!!(val &0x40);
				map6toD   =!!(val &0x80);
				// Super weird special case: value $40 maps to 6toD as well, but write accesses only do not go into state RAM!
				sync();
				break;
		case 0x181:	pageRead  =val &0xF;
				pageWrite =val >>4;
				sync();
				break;
		case 0x182:	frameTimerCount =val;
				frameTimerEnabled =!!val;
				if (frameTimerEnabled) frameTimerCount++;
				break;
		case 0x183:	parallelDataGM =val;
				break;
		case 0x184:	parallelStateGM =val;
				if (parallelStateGM ==0xCB) transferByte =parallelDataGM;
				break;
		case 0x185:	initialized  =!!(val &0x01);
				selectKey    =!!(val &0x02);
				mapCartridge =!!(val &0x04);
				logEnabled   =!!(val &0x08);
				mapSGD       =!!(val &0x10);
				mapWorkRAM   =!!(val &0x20);
				mapEtoF      =!!(val &0x80);
				sync();
				break;
		case 0x186:	logMask =logMask &0x00FF |(val <<8);
				break;
		case 0x187:	logMask =logMask &0xFF00 |val;
				break;
		case 0x188:	logTail =0;
				break;
		case 0x189:	logHead =0;
				break;
		case 0x18A:	break;
		case 0x18B:	break;
		case 0x18D:	inputState =val;
				break;
		case 0x3FF:	
		case 0xFFF:	mapSGD =true;
				sync();
				break;
	}
	cpuWrite_sgd(bank, addr, val);
}

// PPU Cache
void	increaseVRAMAddr (void) {
	if (cache[0] &4) {
		cache[5] +=0x20;
		if ((cache[5] &0xE0) ==0x00) cache[4]++;		
	} else
		if (!++cache[5]) cache[4]++;
}
int	MAPINT	cpuRead_ppuCache (int bank, int addr) {
	int result =cpuRead_sgd(bank, addr);
	switch (addr &7) {
		case 2:	hvToggle =false;
			cache[6] =result &0xFF;
			break;
		case 7:	increaseVRAMAddr();
			break;
	}
	return result;
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
		case 6:	if (hvToggle)
				cache[5] =val;
			else
				cache[4] =val;
			hvToggle =!hvToggle;
			break;
		case 7:	if (cache[4] ==0x3F && !map6toD) {
				int index =cache[5] &0x1F;
				stateRAM.k4[3][0xF00 +index] =val;
				if (!(index &3)) stateRAM.k4[3][0xF00 +(index ^0x10)] =val;
			}
			increaseVRAMAddr();
			break;
	}
	cpuWrite_sgd(bank, addr, val); 
}

// PPU "CIRAM for CHR data" handlers (used in menu)
int	MAPINT	ppuRead_ciram (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr];
}
void	MAPINT	ppuWrite_ciram (int bank, int addr, int val) {
	PPU::PPU[0]->VRAM[bank &1][addr] =val;
}

// Common
void	sync() {
	for (int bank =0x2; bank<=0x3; bank++) {
		SetCPUReadHandler(bank, cpuRead_ppuCache);
		SetCPUReadHandlerDebug(bank, cpuReadDebug_sgd);
		SetCPUWriteHandler(bank, cpuWrite_ppuCache);
	}
	
	SetCPUReadHandler(0x4, cpuRead_reg);
	SetCPUReadHandlerDebug(0x4, cpuReadDebug_reg);
	SetCPUWriteHandler(0x4, cpuWrite_reg);

	SetCPUReadHandler(0x5, cpuRead_ram);
	SetCPUReadHandlerDebug(0x5, cpuRead_ram);
	SetCPUWriteHandler(0x5, mapWorkRAM? cpuWrite_ram: passCPUWrite);
	
	for (int bank =0x6; bank<=0xD; bank++) if (!mapSGD && (map6toD || ciramCHR)) {
		SetCPUReadHandler(bank, cpuRead_prgram);
		SetCPUReadHandlerDebug(bank, cpuRead_prgram);
		SetCPUWriteHandler(bank, cpuWrite_prgram);
	} else {
		SetCPUReadHandler(bank, cpuRead_sgd);
		SetCPUReadHandlerDebug(bank, cpuReadDebug_sgd);
		SetCPUWriteHandler(bank, cpuWrite_log);
	}
	
	for (int bank =0xE; bank<=0xF; bank++) {
		SetCPUReadHandler(bank, cpuRead_EtoF);
		SetCPUReadHandlerDebug(bank, cpuReadDebug_EtoF);
		SetCPUWriteHandler(bank, cpuWrite_log);
	}
	
	for (int bank =0x0; bank<=0xF; bank++) if (ciramCHR) {
		SetPPUReadHandler(bank, ppuRead_ciram);
		SetPPUReadHandlerDebug(bank, ppuRead_ciram);
		SetPPUWriteHandler(bank, ppuWrite_ciram);
	} else {
		SetPPUReadHandler(bank, ppuRead_sgd);
		SetPPUReadHandlerDebug(bank, ppuReadDebug_sgd);
		SetPPUWriteHandler(bank, ppuWrite_sgd);
	}
}

BOOL	MAPINT	load (void) {
	if (enableMasterLink) ml_load();
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

BOOL	MAPINT	loadGM19 (void) {
	loadBIOS (_T("BIOS\\BGM-19.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Game Master (v1.9)");
	enableMasterLink =RI.ROMType !=ROM_FDS && RI.INES_MapperNum !=761;
	v20 =false;
	prgramSize =512*1024;
	chrramSize =256*1024;
	ramMask =0xF;
	return load();
}
BOOL	MAPINT	loadGM20 (void) {
	loadBIOS (_T("BIOS\\BGM-20.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Game Master (v2.0)");
	enableMasterLink =RI.ROMType !=ROM_FDS && RI.INES_MapperNum !=761;
	v20 =true;
	prgramSize =512*1024;
	chrramSize =256*1024;
	ramMask =0xF;
	return load();
}
BOOL	MAPINT	loadMB (void) {
	loadBIOS (_T("BIOS\\BMB.BIN"), bios.b +16384, sizeof(bios.b) -16384);
	Description =_T("Bung Master Boy");
	enableMasterLink =false;
	v20 =false;
	prgramSize =256*1024;
	chrramSize =0;
	ramMask =0x7;
	return load();
}
BOOL	MAPINT	loadGMK (void) {
	loadBIOS (_T("BIOS\\BGMK.BIN"), bios.b +16384, sizeof(bios.b) -16384);
	Description =_T("QJ Game Master Kid");
	enableMasterLink =false;
	prgramSize =32*1024;
	chrramSize =0;
	ramMask =0x0;
	return load();
}
BOOL	MAPINT	loadGAR (void) {
	loadBIOS (_T("BIOS\\QJGAR.BIN"), bios.b +16384, sizeof(bios.b) -16384);
	Description =_T("QJ Game Action Replay");
	enableMasterLink =false;
	prgramSize =32*1024;
	chrramSize =0;
	ramMask =0x0;
	return load();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (enableMasterLink) ml_reset();
	reset_sgd(resetType);
	
	if (resetType ==RESET_HARD) {		
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;
		for (auto& c: stateRAM.b) c =0x00;
		for (auto& c: workRAM) c =0x00;
		for (auto c: cache) c =0x00;
		initialized =false;
		map6toD =false;
		mapEtoF =true;
		mapBIOS =true;
		mapMode =true;
		mapAuxiliary =false;
		mapWorkRAM =false;
		mapSGD =false;
		mapCartridge =false;
		ciramCHR =false;
		
		pageRead =0;
		pageWrite =0;
		parallelDataGM =0;
		parallelStateGM =0;
		parallelDataML =0;
		parallelStateML =0;
		transferByte =0;
		frameTimerEnabled =false;
		frameTimerCount =0;
		hvToggle =false;
		
		intercept =false;
		selectKey =false;
		inputState =0;
		
		logEnabled =false;
		logTail =0;
		logHead =0;
	}
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (enableMasterLink) ml_cpuCycle();
	cpuCycle_sgd();
	if (adMI->CPUCycle) adMI->CPUCycle();	
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	ppuCycle_sgd(addr, scanline, cycle, isRendering);
	if (adMI->PPUCycle) adMI->PPUCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	for (uint32_t i =0; i <prgramSize; i++) SAVELOAD_BYTE(stateMode, offset, data, prgram.b[i]);
	for (uint32_t i =0; i <chrramSize; i++) SAVELOAD_BYTE(stateMode, offset, data, chrram.b[i]);
	for (auto& c: stateRAM.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: workRAM) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, initialized);
	SAVELOAD_BOOL(stateMode, offset, data, map6toD);
	SAVELOAD_BOOL(stateMode, offset, data, mapEtoF);
	SAVELOAD_BOOL(stateMode, offset, data, mapBIOS);
	SAVELOAD_BOOL(stateMode, offset, data, mapMode);
	SAVELOAD_BOOL(stateMode, offset, data, mapAuxiliary);
	SAVELOAD_BOOL(stateMode, offset, data, mapWorkRAM);
	SAVELOAD_BOOL(stateMode, offset, data, mapSGD);
	SAVELOAD_BOOL(stateMode, offset, data, mapCartridge);
	SAVELOAD_BOOL(stateMode, offset, data, ciramCHR);
	
	SAVELOAD_BYTE(stateMode, offset, data, pageRead);
	SAVELOAD_BYTE(stateMode, offset, data, pageWrite);
	
	SAVELOAD_BYTE(stateMode, offset, data, parallelDataGM);
	SAVELOAD_BYTE(stateMode, offset, data, parallelStateGM);
	SAVELOAD_BYTE(stateMode, offset, data, transferByte);
	SAVELOAD_BOOL(stateMode, offset, data, frameTimerEnabled);
	SAVELOAD_BYTE(stateMode, offset, data, frameTimerCount);
	SAVELOAD_BOOL(stateMode, offset, data, hvToggle);
	for (auto& c: cache) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, controllerCount);
	SAVELOAD_BYTE(stateMode, offset, data, controllerData);
	
	SAVELOAD_BOOL(stateMode, offset, data, intercept);
	SAVELOAD_BOOL(stateMode, offset, data, selectKey);
	SAVELOAD_BYTE(stateMode, offset, data, inputState);
	
	SAVELOAD_BOOL(stateMode, offset, data, logEnabled);
	SAVELOAD_WORD(stateMode, offset, data, logTail);
	SAVELOAD_WORD(stateMode, offset, data, logHead);
	SAVELOAD_WORD(stateMode, offset, data, logMask);
	
	offset =saveLoad_sgd(stateMode, offset, data);
	if (enableMasterLink) offset =ml_saveLoad(stateMode, offset, data);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNumGM19 =ID_PLUG_BUNG_GM19;
uint16_t deviceNumGM20 =ID_PLUG_BUNG_GM20;
uint16_t deviceNumGMK =ID_PLUG_BUNG_GMK;
uint16_t deviceNumGAR =ID_PLUG_QJ_GAR;
uint16_t deviceNumMB =ID_PLUG_BUNG_GMB;
} // namespace GameMaster

MapperInfo plugThruDevice_GameMaster19 ={
	&GameMaster::deviceNumGM19,
	_T("Bung Game Master (v1.9)"),
	COMPAT_FULL,
	GameMaster::loadGM19,
	GameMaster::reset,
	NULL,
	GameMaster::cpuCycle,
	GameMaster::ppuCycle,
	GameMaster::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_GameMaster20 ={
	&GameMaster::deviceNumGM20,
	_T("Bung Game Master (v2.0)"),
	COMPAT_FULL,
	GameMaster::loadGM20,
	GameMaster::reset,
	NULL,
	GameMaster::cpuCycle,
	GameMaster::ppuCycle,
	GameMaster::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_MasterBoy ={
	&GameMaster::deviceNumMB,
	_T("Bung Master Boy"),
	COMPAT_FULL,
	GameMaster::loadMB,
	GameMaster::reset,
	NULL,
	GameMaster::cpuCycle,
	GameMaster::ppuCycle,
	GameMaster::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_GameMasterKid ={
	&GameMaster::deviceNumGMK,
	_T("Bung Game Master Kid"),
	COMPAT_FULL,
	GameMaster::loadGMK,
	GameMaster::reset,
	NULL,
	GameMaster::cpuCycle,
	GameMaster::ppuCycle,
	GameMaster::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_GameActionReplay ={
	&GameMaster::deviceNumGAR,
	_T("QJ Game Action Replay"),
	COMPAT_FULL,
	GameMaster::loadGAR,
	GameMaster::reset,
	NULL,
	GameMaster::cpuCycle,
	GameMaster::ppuCycle,
	GameMaster::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice