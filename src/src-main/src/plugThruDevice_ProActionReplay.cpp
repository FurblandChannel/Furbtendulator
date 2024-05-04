#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "PPU.h"
#include "Settings.h"

namespace PlugThruDevice {
namespace ProActionReplay {
union {
	uint8_t	k4 [10][4096];
	uint8_t	k1 [40][1024];
	uint8_t	b  [40*1024];
} bios;
uint8_t	ram[4096];
bool	gameMode;
void	sync();

// Extra RAM at $5000 in both BIOS and game modes
int	MAPINT	cpuRead_ram (int bank, int addr) {
	return ram[addr];
}
void	MAPINT	cpuWrite_ram (int bank, int addr, int val) {
	ram[addr] =val;
}

// Game mode: reading NMI handler switches in PAR BIOS
int	MAPINT	cpuRead_bios_debug (int bank, int addr);
int	MAPINT	cpuRead_nmi (int bank, int addr) {
	if (addr ==0xFFA) {
		gameMode =false;
		sync();
		return cpuRead_bios_debug(bank, addr);
	}
	return passCPURead(bank, addr);
}

// BIOS mode. Reading NMI handler switches to game mode.
int	MAPINT	cpuRead_bios (int bank, int addr) {
	if (bank ==0xF && addr ==0xFFA) {
		gameMode =true;
		sync();
		return passCPURead(bank, addr);
	}
	return bios.k4[bank &7][addr];
}
int	MAPINT	cpuRead_bios_debug (int bank, int addr) {
	return bios.k4[bank &7][addr];
}
int	MAPINT	readCHR_bios (int bank, int addr) {
	return bios.k1[32 +bank][addr];
}
int	MAPINT	readNT_bios (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr &0x3FF];
}
void	MAPINT	writeNT_bios (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[bank &1][addr &0x3FF] =val;
}

void	sync() {
	SetCPUReadHandler(0x5, cpuRead_ram);
	SetCPUReadHandlerDebug(0x5, cpuRead_ram);
	SetCPUWriteHandler(0x5, cpuWrite_ram);
	if (gameMode) {
		for (int bank =0x0; bank<=0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, passPPUWrite);
		}
		for (int bank =0x8; bank<=0xE; bank++) {
			SetCPUReadHandler(bank, passCPURead);
			SetCPUReadHandlerDebug(bank, passCPUReadDebug);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		SetCPUReadHandler(0xF, cpuRead_nmi);
		SetCPUReadHandlerDebug(0xF, passCPUReadDebug);
		SetCPUWriteHandler(0xF, passCPUWrite);
	} else {
		for (int bank =0x0; bank <0x8; bank++) {
			SetPPUReadHandler(bank, readCHR_bios);
			SetPPUReadHandlerDebug(bank, readCHR_bios);
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, cpuRead_bios);
			SetCPUReadHandlerDebug(bank, cpuRead_bios_debug);
			SetCPUWriteHandler(bank, passCPUWrite);
			
			SetPPUReadHandler(bank, readNT_bios);
			SetPPUReadHandlerDebug(bank, readNT_bios);
			SetPPUWriteHandler(bank, writeNT_bios);
		}
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\PAR12B.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Datel Pro Action Replay");
	
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
	if (resetType ==RESET_HARD) for (int i =0; i <sizeof(ram); i++) ram[i] =0x00;
	gameMode =false; // Soft Reset shows BIOS menu
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, gameMode);
	for (auto& c: ram) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_PRO_ACTION_REPLAY;
} // namespace ProActionReplay

MapperInfo plugThruDevice_ProActionReplay ={
	&ProActionReplay::deviceNum,
	_T("Datel Pro Action Replay"),
	COMPAT_FULL,
	ProActionReplay::load,
	ProActionReplay::reset,
	NULL,
	NULL,
	NULL,
	ProActionReplay::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice