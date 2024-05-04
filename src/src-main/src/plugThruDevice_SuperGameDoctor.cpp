#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "CPU.h"
#include "NES.h"
#include "PPU.h"
#include "FDS.h"
#include "Settings.h"

namespace PlugThruDevice {
namespace BungSuperGameDoctor {
union {
	uint8_t	k1 [8][1024];
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} bios;
union {
	uint8_t p8 [64][8192];
	uint8_t k8 [64][2][4096];
	uint8_t k16[32][4][4096];
	uint8_t k32[16][8][4096];
	uint8_t	b  [512*1024];
} prgram;
union {
	uint8_t k8 [4][8][1024];
	uint8_t b  [32*1024];
} chrram;

#define		GDSTATE_FDS 0
#define 	GDSTATE_LOAD 1
#define 	GDSTATE_PLAY 2
uint8_t		prgMask;
uint8_t		state;
uint8_t		modeGD;
uint8_t		latchGD;
uint8_t		chrGD;
bool		protectPRG;
uint8_t		mirroring;
uint8_t		prgLoad;
uint8_t		prgGD[4];
	
bool		prgModeMC;
uint8_t		prgMC[4];

int16_t		sgdCounter;

void		sync();

// Fastload callbacks
void	MAPINT	ppuWrite_chr (int bank, int addr, int val);
void	cbSkipGDHeader (void) {
	FDS::putCPUByte(0x2001, 0x00);
	FDS::putCPUByte(0x99, FDS::getCPUByte(0x87));
	FDS::putCPUByte(0x96, 0x00);
	FDS::putCPUByte(0x9A, 0x00);
	FDS::putCPUByte(0x06, FDS::getCPUByte(0x06) -1);
	CPU::CPU[0]->PC =0xF41D;
}

void	cbReadFileHeader (void) {
	std::vector<uint8_t> header =FDS::diskReadBlock(3, 15);
	if (FDS::lastErrorCode ==0) for (int i =10; i <15; i++) FDS::putCPUByte(0x88 +i -10, header[i]);
	CPU::CPU[0]->IN_RTS();
}
void	cbReadFileCPU (void) {
	std::vector<uint8_t> data =FDS::diskReadBlock(4, 32768);
	if (FDS::lastErrorCode ==0) {	
		int i =0;
		int addr =FDS::getCPUWord(0x0088);
		do {
			FDS::putCPUByte(addr++, data[i++]);
			if (addr ==0xE000) addr =0x6000;
		} while (addr !=0x8000);
		FDS::putCPUWord(0x0088, addr);
	}
	CPU::CPU[0]->PC =prgMask ==0x3F? 0xF480: 0xE105;
}
void	cbReadFilePPU (void) {
	uint8_t reg43FF =FDS::getCPUByte(0x90);
	int sizes[8] ={ 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x4000, 0x2000 };
	int size =sizes[FDS::getCPUByte(0x81) >>5];
	std::vector<uint8_t> data =FDS::diskReadBlock(4, size);
	if (FDS::lastErrorCode ==0) {	
		FDS::putCPUByte(0x2001, 0x00);
		for (int i =0; i <size; i++) {
			int addr =i &0x1FFF;
			if (addr ==0) FDS::putCPUByte(0x43FF, ++reg43FF);
			ppuWrite_chr(addr >>10, addr &0x3FF, data[i]);
		}
		FDS::putCPUByte(0x90, reg43FF);
	}
	CPU::CPU[0]->PC =0xE105;
}
void	cbReadFFETrainer (void) {
	std::vector<uint8_t> data =FDS::diskReadBlock(5, 512);
	if (FDS::lastErrorCode ==0) for (int i =0; i <512; i++) FDS::putCPUByte(0x5000 +i, data[i]);
	CPU::CPU[0]->PC =prgMask ==0x3F? 0xF4B5: 0xF4AE;
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
	CPU::CPU[0]->PC =prgMask ==0x3F? 0xF640: 0xF5B3;
}
// FDS state
void	MAPINT	cpuWrite_wakeUp (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	if (addr ==0x3FF || addr ==0xFFF) { // E53C entry point
		if (Settings::FastLoad) {
			CPU::CPU[0]->PC =0xE53C;
			CPU::callbacks.clear();
			CPU::callbacks.push_back({0, 0xF400, cbSkipGDHeader});
			CPU::callbacks.push_back({0, 0xE050, cbReadFileHeader});
			if (prgMask ==0x3F)
				CPU::callbacks.push_back({0, 0xF4F6, cbReadFileCPU});
			else
				CPU::callbacks.push_back({0, 0xE0A7, cbReadFileCPU});
			CPU::callbacks.push_back({0, 0xE0C8, cbReadFilePPU});
			CPU::callbacks.push_back({0, 0xE658, cbNextSide});
			CPU::callbacks.push_back({0, prgMask ==0x3F? (uint16_t) 0xF496: (uint16_t) 0xF48F, cbReadFFETrainer});
		}
		state =GDSTATE_LOAD;
		sync();
	}
}

// Loading state
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios.k4[bank &1][addr];
}
int	MAPINT	ppuRead_bios (int bank, int addr) {
	return bios.k1[4][addr];
}

void	MAPINT	cpuWrite_fdsram (int bank, int addr, int val) {
	passCPUWrite(bank &1 |6, addr, val);
}

int	MAPINT	cpuRead_load (int bank, int addr) {
	return prgram.k32[prgLoad &(prgMask >>2)][bank &7][addr];
}
void	MAPINT	cpuWrite_load (int bank, int addr, int val) {
	prgram.k32[prgLoad &(prgMask >>2)][bank &7][addr] =val;
}

// Loading and playing state
void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	switch (addr) {
		case 0x100:
			sgdCounter =val;
			pdSetIRQ(1);
			break;
		case 0x101:
			sgdCounter |=val <<8;
			pdSetIRQ(1);
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			state =GDSTATE_PLAY;
			CPU::callbacks.clear();
			modeGD =val >>5;
			protectPRG =!!(addr &2);
			mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);			
			sync();
			break;
		case 0x3FE: case 0x3FF:
			chrGD =val &3;
			prgModeMC =!(addr &1);
			latchGD =val;
			prgLoad =((val &0x30) >>4) |
				  ((val &0x08) >>1) |
				  ((val &0x04) <<1);
			sync();
			break;
	}
}

int	MAPINT	ppuRead_chr (int bank, int addr) {
	return chrram.k8[chrGD][bank][addr];
}

void	MAPINT	ppuWrite_chr (int bank, int addr, int val) {
	if (state !=GDSTATE_PLAY || modeGD <4) chrram.k8[chrGD][bank][addr] =val;
	if (state ==GDSTATE_FDS) passPPUWrite(bank, addr, val);
}

// Playing state
int	MAPINT	cpuRead_gd (int bank, int addr) {
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
		prgram.k8[prgGD[index]][bank &1][addr] =val;
	}
}
int	MAPINT	cpuRead_mc (int bank, int addr) {
	int index =(bank &7) >>1;
	return prgram.k8[prgMC[index] &prgMask][bank &1][addr];
}
void	MAPINT	cpuWrite_mc (int bank, int addr, int val) {
	if (protectPRG)
		cpuWrite_gd (bank, addr, val);
	else {
		int index =(bank &7) >>1;
		prgram.k8[prgMC[index] &prgMask][bank &1][addr] =val;
	}
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
	if (bank !=0xF || addr <0xF00)
		PPU::PPU[0]->VRAM[bank &1][addr] =val;
}
void	MAPINT	writeNT_H (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00)
		PPU::PPU[0]->VRAM[(bank >>1) &1][addr] =val;
}
void	MAPINT	writeNT_0 (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00)
		PPU::PPU[0]->VRAM[0][addr] =val;
}
void	MAPINT	writeNT_1 (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00)
		PPU::PPU[0]->VRAM[1][addr] =val;
}

// Common
void	sync() {
	switch(state) {
		case GDSTATE_FDS:
			SetCPUWriteHandler(0x4, cpuWrite_wakeUp);
			for (int bank =0x6; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, passCPURead);
				SetCPUReadHandlerDebug(bank, passCPUReadDebug);
				SetCPUWriteHandler(bank, passCPUWrite);
			}
			for (int bank =0x0; bank<=0x7; bank++) {
				SetPPUReadHandler(bank, passPPURead);
				SetPPUReadHandlerDebug(bank, passPPUReadDebug);
				SetPPUWriteHandler(bank, ppuWrite_chr);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				SetPPUReadHandler(bank, passPPURead);
				SetPPUReadHandlerDebug(bank, passPPUReadDebug);
				SetPPUWriteHandler(bank, passPPUWrite);
			}
			break;
		case GDSTATE_LOAD:
			SetCPUWriteHandler(0x4, cpuWrite_reg);
			SetCPUWriteHandler(0x5, cpuWrite_fdsram); // Not sure if this is correct. It is required for Magic Card games with BIOS rev0 and rev1, but given that later BIOS revisions write to the $Fxxx range instead of the $5xxx range, it may well be a BIOS bug in the earlier revisions.
			for (int bank =0x6; bank<=0xD; bank++) {
				SetCPUReadHandler(bank, cpuRead_load);
				SetCPUReadHandlerDebug(bank, cpuRead_load);
				SetCPUWriteHandler(bank, cpuWrite_load);
			}
			for (int bank =0xE; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, cpuRead_bios);
				SetCPUReadHandlerDebug(bank, cpuRead_bios);
				SetCPUWriteHandler(bank, cpuWrite_fdsram);
			}
			for (int bank =0x0; bank<=0x7; bank++) {
				SetPPUReadHandler(bank, ppuRead_chr);
				SetPPUReadHandlerDebug(bank, ppuRead_chr);
				SetPPUWriteHandler(bank, ppuWrite_chr);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				SetPPUReadHandler(bank, passPPURead);
				SetPPUReadHandlerDebug(bank, passPPUReadDebug);
				SetPPUWriteHandler(bank, passPPUWrite);
			}
			break;
		case GDSTATE_PLAY:
			switch(modeGD) {
				case 0:	prgGD[0] =((latchGD &7) <<1) |0;
					prgGD[1] =((latchGD &7) <<1) |1;
					prgGD[2] =14;
					prgGD[3] =15;
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
					break;
			}
			for (int bank =0x0; bank <0x8; bank++) {
				SetPPUReadHandler(bank, ppuRead_chr);
				SetPPUReadHandlerDebug(bank, ppuRead_chr);
				SetPPUWriteHandler(bank, ppuWrite_chr);
			}
			SetCPUWriteHandler(0x4, cpuWrite_reg);
			
			for (int bank =0x5; bank<=0x7; bank++) {
				SetCPUReadHandler(bank, passCPURead);
				SetCPUReadHandlerDebug(bank, passCPUReadDebug);
				SetCPUWriteHandler(bank, passCPUWrite);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
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
			break;
	}
}

BOOL	MAPINT	loadSGD2M (void) {
	loadBIOS (_T("BIOS\\BSGD2M.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Super Game Doctor 2M");
	prgMask =0x1F;
	
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}
BOOL	MAPINT	loadSGD4M (void) {
	loadBIOS (_T("BIOS\\BSGD4M.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Super Game Doctor 4M");
	prgMask =0x3F;
	
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}
BOOL	MAPINT	loadVGC2M (void) {
	loadBIOS (_T("BIOS\\VGC2M.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Venus Game Converter 2M");
	prgMask =0x1F;
	
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->CPUCycle) MI->CPUCycle =adMI->CPUCycle;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) {
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;
		state =GDSTATE_FDS;
		modeGD =0;
		latchGD =0;
		chrGD =0;
		protectPRG =true;
		mirroring =2;
		prgLoad =0;
		for (int i =0; i <4; i++) prgGD[i] =i +12;
		
		prgModeMC =false;
		for (auto c: prgMC) c =0;
	}
	sgdCounter =0;
	pdSetIRQ(1);
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (adMI->CPUCycle) adMI->CPUCycle();
	if (state ==GDSTATE_PLAY && sgdCounter <0 && !++sgdCounter) pdSetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, state);
	SAVELOAD_BYTE(stateMode, offset, data, modeGD);
	SAVELOAD_BYTE(stateMode, offset, data, latchGD);
	SAVELOAD_BYTE(stateMode, offset, data, chrGD);
	SAVELOAD_BOOL(stateMode, offset, data, protectPRG);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, prgLoad);
	for (auto& c: prgGD) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	SAVELOAD_BOOL(stateMode, offset, data, prgModeMC);
	for (auto& c: prgMC) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	for (int i =0; i <=prgMask; i++) for (int j =0; j <8192; j++) SAVELOAD_BYTE(stateMode, offset, data, prgram.p8[i][j]);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	SAVELOAD_WORD(stateMode, offset, data, sgdCounter);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNumGC2M =ID_PLUG_VENUS_GC2M;
uint16_t deviceNumSGD2M =ID_PLUG_BUNG_SGD2M;
uint16_t deviceNumSGD4M =ID_PLUG_BUNG_SGD4M;
} // namespace BungSuperGameDoctor

MapperInfo plugThruDevice_SuperGameDoctor2M ={
	&BungSuperGameDoctor::deviceNumSGD2M,
	_T("Bung Super Game Doctor 2M"),
	COMPAT_FULL,
	BungSuperGameDoctor::loadSGD2M,
	BungSuperGameDoctor::reset,
	NULL,
	BungSuperGameDoctor::cpuCycle,
	NULL,
	BungSuperGameDoctor::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_SuperGameDoctor4M ={
	&BungSuperGameDoctor::deviceNumSGD4M,
	_T("Bung Super Game Doctor 4M"),
	COMPAT_FULL,
	BungSuperGameDoctor::loadSGD4M,
	BungSuperGameDoctor::reset,
	NULL,
	BungSuperGameDoctor::cpuCycle,
	NULL,
	BungSuperGameDoctor::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_GameConverter2M ={
	&BungSuperGameDoctor::deviceNumGC2M,
	_T("Venus Game Converter 2M"),
	COMPAT_FULL,
	BungSuperGameDoctor::loadVGC2M,
	BungSuperGameDoctor::reset,
	NULL,
	NULL,
	NULL,
	BungSuperGameDoctor::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice