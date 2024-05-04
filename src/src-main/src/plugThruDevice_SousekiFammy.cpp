#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "CPU.h"
#include "PPU.h"
#include "FDS.h"
#include "Settings.h"

namespace PlugThruDevice {
namespace SousekiFammy {
union {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} bios;
union {
	uint8_t k32[8][4096];
	uint8_t	b  [32*1024];
} prgram;
union {
	uint8_t k1 [8][1024];
	uint8_t b  [8*1024];
} chrram;
union {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
} workRAM;

uint8_t	mode;
bool	horizontalMirroring;
void	sync();

int	MAPINT	cpuRead_APU (int bank, int addr) {
	return FDS::readReg(bank, addr);
}
int	MAPINT	cpuReadDebug_APU (int bank, int addr) {
	return FDS::readRegDebug(bank, addr);
}
void	MAPINT	cpuWrite_mode (int bank, int addr, int val) {
	if (addr ==0xC00) {
		mode =val;
		sync();
	} else {
		if (addr ==0x25) {
			horizontalMirroring =!!(val &0x08);
			sync();
		}
		FDS::writeReg(bank, addr, val);
	}
}
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios.k4[bank &1][addr];
}
int	MAPINT	cpuRead_workRAM (int bank, int addr) {
	return workRAM.k4[bank &1][addr];
}
void	MAPINT	cpuWrite_workRAM (int bank, int addr, int val) {
	workRAM.k4[bank &1][addr] =val;
}
int	MAPINT	cpuRead_prgRAM (int bank, int addr) {
	return prgram.k32[bank &7][addr];
}

void	MAPINT	cpuWrite_prgRAM (int bank, int addr, int val) {
	prgram.k32[bank &7][addr] =val;
}

int	MAPINT	ppuRead_chr (int bank, int addr) {
	return chrram.k1[bank][addr];
}
void	MAPINT	ppuWrite_chr (int bank, int addr, int val) {
	chrram.k1[bank][addr] =val;
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

void	sync (void) {
	SetCPUReadHandler(0x4, cpuRead_APU);
	SetCPUReadHandlerDebug(0x4, cpuReadDebug_APU);
	SetCPUWriteHandler(0x4, cpuWrite_mode);
	
	SetCPUReadHandler(0x5, cpuRead_workRAM);
	SetCPUReadHandlerDebug(0x5, cpuRead_workRAM);
	SetCPUWriteHandler(0x5, cpuWrite_workRAM);

	for (int bank =0x6; bank <=0xD; bank++) {
		SetCPUReadHandler     (bank, mode ==0x80? passCPURead:      cpuRead_prgRAM);
		SetCPUReadHandlerDebug(bank, mode ==0x80? passCPUReadDebug: cpuRead_prgRAM);
		SetCPUWriteHandler    (bank, mode ==0x14? ignoreWrite:      cpuWrite_prgRAM);
	}
	for (int bank =0xE; bank <=0xF; bank++) {
		if (mode ==0x14 || mode ==0x84) {
			SetCPUReadHandler(bank, cpuRead_workRAM);
			SetCPUReadHandlerDebug(bank, cpuRead_workRAM);
			SetCPUWriteHandler(bank, mode ==0x14? ignoreWrite: cpuWrite_workRAM);
		} else {
			SetCPUReadHandler(bank, cpuRead_bios);
			SetCPUReadHandlerDebug(bank, cpuRead_bios);
			SetCPUWriteHandler(bank, ignoreWrite);
		}
	}
	for (int bank =0x0; bank <0x8; bank++) {
		SetPPUReadHandler     (bank, mode ==0xE1? passPPURead:      ppuRead_chr);
		SetPPUReadHandlerDebug(bank, mode ==0xE1? passPPUReadDebug: ppuRead_chr);
		SetPPUWriteHandler    (bank, mode ==0x14? ignoreWrite:      ppuWrite_chr);
	}
	for (int bank =0x8; bank<=0xF; bank++) {
		SetPPUReadHandler     (bank, horizontalMirroring? readNT_H:  readNT_V);
		SetPPUReadHandlerDebug(bank, horizontalMirroring? readNT_H:  readNT_V);
		SetPPUWriteHandler    (bank, horizontalMirroring? writeNT_H: writeNT_V);
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\DISKSYS.ROM"), bios.b, sizeof(bios.b));
	Description =_T("Souseki Fammy");
	if (RI.ROMType !=ROM_FDS) FDS::load(false);
	
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;	
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	EnableMenuItem(hMenu, ID_FILE_REPLACE, MF_ENABLED);
	adMI->Reset(resetType);
	if (RI.ROMType !=ROM_FDS) FDS::reset(RESET_HARD);
	if (resetType ==RESET_HARD) {
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;
		for (auto& c: workRAM.b) c =0x00;
		mode =0;
		horizontalMirroring =false;
	}
	CPU::callbacks.clear();
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (RI.ROMType !=ROM_FDS) FDS::cpuCycle();
	if (adMI->CPUCycle) adMI->CPUCycle();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_SOUSEIKI_FAMMY;
} // namespace SousekiFammy

MapperInfo plugThruDevice_SousekiFammy ={
	&SousekiFammy::deviceNum,
	_T("Souseki Fammy"),
	COMPAT_FULL,
	SousekiFammy::load,
	SousekiFammy::reset,
	NULL,
	SousekiFammy::cpuCycle,
	NULL,
	SousekiFammy::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice



/*	5800-8000 -> 5000-7800
	D800-E000 -> 7800-8000


	5000-77FF -> D800-FFFF
	
	=> 8000-D7FF stays
	
	
*/