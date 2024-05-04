#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "CPU.h"
#include "PPU.h"
#include "Settings.h"

namespace PlugThruDevice {
namespace MagicCard {
union {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} bios;
union {
	uint8_t k8 [32][2][4096];
	uint8_t k16[16][4][4096];
	uint8_t k32[ 8][8][4096];
	uint8_t	b  [256*1024];
} prgram;
union {
	uint8_t k8 [4][8][1024];
	uint8_t b  [32*1024];
} chrram;
union {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} wram;

#define	GDSTATE_FDS 0
#define GDSTATE_LOAD 1
#define GDSTATE_PLAY 2
uint8_t	state;
uint8_t	mode;
uint8_t	latch;
uint8_t	chr;
bool	protectPRG;
bool	protectCHR;
uint8_t	mirroring;
uint8_t	pageLoad;
uint8_t	page8;
uint8_t	pageC;

bool	prgModeMC;
uint8_t	prgMC[4];

void	sync();

// FDS state
void	MAPINT	cpuWrite_wakeUp (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	if (addr ==0x3FF || addr ==0xFFF) {
		state =GDSTATE_LOAD;
		sync();
	}
}

// Loading state
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios.k4[bank &1][addr];
}

int	MAPINT	cpuRead_wram (int bank, int addr) {
	return wram.k4[bank &1][addr];
}
void	MAPINT	cpuWrite_wram (int bank, int addr, int val) {
	wram.k4[bank &1][addr] =val;
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
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			state =GDSTATE_PLAY;
			mode =val >>5;
			protectPRG =!!(addr &2);
			mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);
			
			// Modes 1/4-7 change CHR write-protect status as soon as they are set
			if (mode ==1)
				protectCHR =false;
			else
			if (mode >=4)
				protectCHR =true;
			
			sync();
			break;
		case 0x3FE: case 0x3FF:
			chr =val &3;
			prgModeMC =!(addr &1);
			pageLoad =((val &0x30) >>4) |
				  ((val &0x08) >>1) |
				  ((val &0x04) <<1);
			sync();
			break;
	}
}

int	MAPINT	ppuRead_chr (int bank, int addr) {
	return chrram.k8[chr][bank][addr];
}

void	MAPINT	ppuWrite_chr (int bank, int addr, int val) {
	if (state ==GDSTATE_LOAD || !protectCHR) chrram.k8[chr][bank][addr] =val;
}

// Playing state
int	MAPINT	cpuRead_gd (int bank, int addr) {
	if (bank <0xC)
		return prgram.k16[page8][bank &3][addr];
	else
		return prgram.k16[pageC][bank &3][addr];
}
void	MAPINT	cpuWrite_gd (int bank, int addr, int val) {
	if (protectPRG) {
		latch =val;
		prgMC[(bank &7) >>1] =(val >>2) &0x1F;
		
		// Modes 0/2/3 change CHR write-protect status only once the latch is written to
		if (mode ==0 || mode ==2 || mode ==3) protectCHR =false;
		
		sync();
	} else
	if (bank <0xC)
		prgram.k16[page8][bank &3][addr] =val;
	else
		prgram.k16[pageC][bank &3][addr] =val;
}
int	MAPINT	cpuRead_mc (int bank, int addr) {
	int index =(bank &7) >>1;
	return prgram.k8[prgMC[index]][bank &1][addr];
}
void	MAPINT	cpuWrite_mc (int bank, int addr, int val) {
	if (protectPRG)
		cpuWrite_gd (bank, addr, val);
	else {
		int index =(bank &7) >>1;
		prgram.k8[prgMC[index]][bank &1][addr] =val;
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
			for (int bank =0x0; bank <0xF; bank++) {
				SetPPUReadHandler(bank, passPPURead);
				SetPPUReadHandlerDebug(bank, passPPUReadDebug);
				SetPPUWriteHandler(bank, passPPUWrite);
			}
			SetCPUWriteHandler(0x4, cpuWrite_wakeUp);
			for (int bank =0x6; bank<=0xF; bank++) {
				SetCPUReadHandler(bank, passCPURead);
				SetCPUReadHandlerDebug(bank, passCPUReadDebug);
				SetCPUWriteHandler(bank, passCPUWrite);
			}
			break;
		case GDSTATE_LOAD:
			for (int bank =0x0; bank <0xF; bank++) {
				SetPPUReadHandler(bank, bank <0x8? ppuRead_chr: passPPURead);
				SetPPUReadHandlerDebug(bank, bank <0x8? ppuRead_chr: passPPURead);
				SetPPUWriteHandler(bank, bank <0x8? ppuWrite_chr: passPPUWrite);
			}
			SetCPUWriteHandler(0x4, cpuWrite_reg);
			
			SetCPUReadHandler(0x5, cpuRead_wram);
			SetCPUReadHandlerDebug(0x5, cpuRead_wram);
			SetCPUWriteHandler(0x5, cpuWrite_wram);
			
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
			break;
		case GDSTATE_PLAY:
			switch(mode) {
				case 0:	page8 =latch &7;
					pageC =7;
					chr =0;
					break;
				case 1: page8 =(latch >>2) &15;
					pageC =7;
					chr =latch &3;
					break;
				case 2:	page8 =latch &15;
					pageC =15;
					chr =0;
					break;
				case 3:	page8 =15;
					pageC =latch &15;
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
			}
			for (int bank =0x0; bank <0x8; bank++) {
				SetPPUReadHandler(bank, ppuRead_chr);
				SetPPUReadHandlerDebug(bank, ppuRead_chr);
				SetPPUWriteHandler(bank, ppuWrite_chr);
			}
			SetCPUWriteHandler(0x4, cpuWrite_reg);
			
			SetCPUReadHandler(0x5, passCPURead);
			SetCPUReadHandlerDebug(0x5, passCPUReadDebug);
			SetCPUWriteHandler(0x5, passCPUWrite);
			
			for (int bank =0x6; bank<=0x7; bank++) {
				SetCPUReadHandler(bank, cpuRead_wram);
				SetCPUReadHandlerDebug(bank, cpuRead_wram);
				SetCPUWriteHandler(bank, cpuWrite_wram);
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

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\TCMC2T.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Turbo Co. MagicCard II Turbo");
	
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
		for (auto& c: wram.b) c =0x00;
		state =GDSTATE_FDS;
		mode =0;
		latch =0;
		protectPRG =true;
		protectCHR =false;
		mirroring =2;
		chr =0;
		pageLoad =0;
		page8 =0;
		pageC =1;
		
		prgModeMC =false;
		for (auto c: prgMC) c =0;
	}
	CPU::callbacks.clear();
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, state);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	SAVELOAD_BOOL(stateMode, offset, data, protectPRG);
	SAVELOAD_BOOL(stateMode, offset, data, protectCHR);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, pageLoad);
	SAVELOAD_BYTE(stateMode, offset, data, page8);
	SAVELOAD_BYTE(stateMode, offset, data, pageC);
	
	SAVELOAD_BOOL(stateMode, offset, data, prgModeMC);
	for (auto& c: prgMC) SAVELOAD_BYTE(stateMode, offset, data, c);
	
	for (auto& c: prgram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: wram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_TC_MC2T;
} // namespace MagicCard

MapperInfo plugThruDevice_MagicardIITurbo ={
	&MagicCard::deviceNum,
	_T("Turbo Co. MagicCard II Turbo"),
	COMPAT_FULL,
	MagicCard::load,
	MagicCard::reset,
	NULL,
	NULL,
	NULL,
	MagicCard::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice