#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "PPU.h"

#include "Settings.h"
#include "CPU.h"
#include "NES.h"
#include "FDS.h"

namespace PlugThruDevice {
namespace GameDoctor {
union {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} bios;
union {
	uint8_t k16[8][4][4096];
	uint8_t k32[4][8][4096];
	uint8_t	b  [128*1024];
} prgram;
union {
	uint8_t k8 [4][8][1024];
	uint8_t b  [32*1024];
} chrram;

#define	GDSTATE_FDS 0
#define GDSTATE_LOAD 1
#define GDSTATE_PLAY 2
uint8_t	state;
uint8_t	mode;
uint8_t	latch;
uint8_t	chr;
bool	horizontalMirroring;
uint8_t	pageLoad;
uint8_t	page8;
uint8_t	pageC;

void	sync();

// Fastload callbacks
void	MAPINT	ppuWrite_chr (int bank, int addr, int val);
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
	CPU::CPU[0]->PC =0xE105;
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
	CPU::CPU[0]->PC =0xE569;
}
void	cbCopyProtect (void) {
	CPU::CPU[0]->IN_RTS();
}

// FDS state
void	MAPINT	cpuWrite_wakeUp (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	if (addr ==0x3FF || addr ==0xFFF) {
		state =GDSTATE_LOAD;
		if (Settings::FastLoad) {
			CPU::CPU[0]->PC =0xE54E;
			CPU::callbacks.clear();
			CPU::callbacks.push_back({0, 0xE050, cbReadFileHeader});
			CPU::callbacks.push_back({0, 0xE0A7, cbReadFileCPU});
			CPU::callbacks.push_back({0, 0xE0C8, cbReadFilePPU});
			CPU::callbacks.push_back({0, 0xE489, cbCopyProtect});
			CPU::callbacks.push_back({0, 0xE658, cbNextSide});
		}
		sync();
	}
}

// Loading state
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios.k4[bank &1][addr];
}

int	MAPINT	cpuRead_load (int bank, int addr) {
	return prgram.k32[pageLoad][bank &7][addr];
}

void	MAPINT	cpuWrite_load (int bank, int addr, int val) {
	prgram.k32[pageLoad][bank &7][addr] =val;
}

// Loading and playing state
void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	switch (addr) {
		case 0x2FF:
			state =GDSTATE_PLAY;
			CPU::callbacks.clear();
			mode =val >>5;
			horizontalMirroring =!!(val &0x10);
			sync();
			break;
		case 0x3FF:
			chr =val &3;
			pageLoad =(val >>4) &3;
			sync();
			break;
	}
}

int	MAPINT	ppuRead_chr (int bank, int addr) {
	return chrram.k8[chr][bank][addr];
}

void	MAPINT	ppuWrite_chr (int bank, int addr, int val) {
	if (state !=GDSTATE_PLAY || mode <4) chrram.k8[chr][bank][addr] =val;
	if (state ==GDSTATE_FDS) passPPUWrite(bank, addr, val);
}

// Playing state
int	MAPINT	cpuRead_play (int bank, int addr) {
	if (bank <0xC)
		return prgram.k16[page8 &7][bank &3][addr];
	else
		return prgram.k16[pageC &7][bank &3][addr];
}
void	MAPINT	cpuWrite_play (int bank, int addr, int val) {
	latch =val;
	sync();
}
int	MAPINT	readNT_V (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr &0x3FF];
}
int	MAPINT	readNT_H (int bank, int addr) {
	return PPU::PPU[0]->VRAM[(bank >>1) &1][addr &0x3FF];
}
void	MAPINT	writeNT_V (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[bank &1][addr &0x3FF] =val;
}
void	MAPINT	writeNT_H (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[(bank >>1) &1][addr &0x3FF] =val;
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
			for (int bank =0x6; bank<=0xD; bank++) {
				SetCPUReadHandler(bank, cpuRead_load);
				SetCPUReadHandlerDebug(bank, cpuRead_load);
				SetCPUWriteHandler(bank, cpuWrite_load);
			}
			for (int bank =0xE; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, cpuRead_bios);
				SetCPUReadHandlerDebug(bank, cpuRead_bios);
				SetCPUWriteHandler(bank, passCPUWrite);
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
			switch(mode) {
				case 2:	// I007
					page8 =0;
					pageC =1;
					chr =0;
					break;
				case 4:	page8 =(latch >>3) &0x06;
					pageC =(latch >>3) &0x06 |1;
					chr =latch &3;
					break;
				case 5:	page8 =6;
					pageC =7;
					chr =latch &3;
					break;
				case 6:	page8 =6;
					pageC =7;
					chr =latch &1;
					break;
				case 7:	page8 =6;
					pageC =7;
					chr =0;
					break;
				default:page8 =latch &7;
					pageC =7;
					chr =0;
					break;
			}
			SetCPUWriteHandler(0x4, cpuWrite_reg);
			for (int bank =0x6; bank<=0x7; bank++) {
				SetCPUReadHandler(bank, passCPURead);
				SetCPUReadHandlerDebug(bank, passCPUReadDebug);
				SetCPUWriteHandler(bank, passCPUWrite);
			}
			for (int bank =0x0; bank <0x8; bank++) {
				SetPPUReadHandler(bank, ppuRead_chr);
				SetPPUReadHandlerDebug(bank, ppuRead_chr);
				SetPPUWriteHandler(bank, ppuWrite_chr);
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, cpuRead_play);
				SetCPUReadHandlerDebug(bank, cpuRead_play);
				SetCPUWriteHandler(bank, cpuWrite_play);
				
				SetPPUReadHandler     (bank, horizontalMirroring? readNT_H:  readNT_V);
				SetPPUReadHandlerDebug(bank, horizontalMirroring? readNT_H:  readNT_V);
				SetPPUWriteHandler    (bank, horizontalMirroring? writeNT_H: writeNT_V);
			}
			break;
	}
}

BOOL	MAPINT	loadBGD1M (void) {
	loadBIOS (_T("BIOS\\BGD1M.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Game Doctor");
		
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->CPUCycle) MI->CPUCycle =adMI->CPUCycle;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

BOOL	MAPINT	loadVGC1M (void) {
	if (!loadBIOS (_T("BIOS\\VGC1M.BIN"), bios.b, sizeof(bios.b))) return FALSE;
	Description =_T("Venus Game Converter 1M");
	
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
		mode =0;
		latch =0;
		horizontalMirroring =false;
		chr =0;
		pageLoad =0;
		page8 =0;
		pageC =1;
	}
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, state);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	SAVELOAD_BYTE(stateMode, offset, data, pageLoad);
	SAVELOAD_BYTE(stateMode, offset, data, page8);
	SAVELOAD_BYTE(stateMode, offset, data, pageC);
	for (auto& c: prgram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNumBGD1M =ID_PLUG_BUNG_GD1M;
uint16_t deviceNumVGC1M =ID_PLUG_VENUS_GC1M;
} // namespace GameDoctor

MapperInfo plugThruDevice_GameDoctor1M ={
	&GameDoctor::deviceNumBGD1M,
	_T("Bung Game Doctor"),
	COMPAT_FULL,
	GameDoctor::loadBGD1M,
	GameDoctor::reset,
	NULL,
	NULL,
	NULL,
	GameDoctor::saveLoad,
	NULL,
	NULL
};
MapperInfo plugThruDevice_GameConverter1M ={
	&GameDoctor::deviceNumVGC1M,
	_T("Venus Game Converter 1M"),
	COMPAT_FULL,
	GameDoctor::loadVGC1M,
	GameDoctor::reset,
	NULL,
	NULL,
	NULL,
	GameDoctor::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice