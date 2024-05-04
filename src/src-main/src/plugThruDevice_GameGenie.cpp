#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "CPU.h"
#include "PPU.h"

namespace PlugThruDevice {
namespace GameGenie {
uint8_t	bios[4096];
	
struct	Code {
	bool		enable;
	bool		compare;
	uint16_t	addr;
	uint8_t		compareValue;
	uint8_t		replaceValue;
};
Code	codes[3];
bool	gameMode;
void	sync();

// Game Mode: intercept CPU reads
int	MAPINT	cpuRead_game (int bank, int addr) {
	int value =adCPURead[bank](bank, addr);
	for (auto& code: codes) {
		if (code.enable && code.addr ==((bank <<12) |addr)) {
			if (!code.compare || value ==code.compareValue) {
				value =code.replaceValue;
				/* When a cartridge has something mapped at $4020-$7FFF (WRAM, PRG ROM) and a code for region $C020-$FFFF is added,
				   the Game Genie will hold the slave cartridge's /ROMSEL at 1 when reading from that location.
				   But then, the cartridge logic will see this read cycle as something below $8000, enabling the chip that is mapped here,
				   causing bus conflict at this location and resulting in invalid data being returned to the CPU. */
				if ((bank >=0xC && addr >=0x020 || bank >0xC) && CPU::CPU[bank >>4]->Readable[bank &0x7]) value &=(EI.GetCPUReadHandler(bank &7))(bank &7, addr);
			}
		}
	}
	return value;
}

int	MAPINT	cpuReadDebug_game (int bank, int addr) {
	int value =adCPUReadDebug[bank](bank, addr);
	for (auto& code: codes) {
		if (code.enable && code.addr ==((bank <<12) |addr)) {
			if (!code.compare || value ==code.compareValue) value =code.replaceValue;
		}
	}
	return value;
}

// BIOS mode
int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios[addr];
}

void	MAPINT	cpuWrite_bios (int bank, int addr, int val) {
	int which =(addr -1) >>2;
	switch(addr) {
		case 0:	gameMode =!!(val &0x01);
			codes[0].compare =!!(val &0x02);
			codes[1].compare =!!(val &0x04);
			codes[2].compare =!!(val &0x08);
			codes[0].enable  = !(val &0x10);
			codes[1].enable  = !(val &0x20);
			codes[2].enable  = !(val &0x40);
			sync();
			break;
		case 1:	case 5: case 9:
			codes[which].addr &=~0x7F00;
			codes[which].addr |=(val &0x7F) <<8;
			break;
		case 2:	case 6: case 10:
			codes[which].addr &=~0x00FF;
			codes[which].addr |=val;
			break;
		case 3:	case 7:	case 11:
			codes[which].compareValue =val;
			break;
		case 4: case 8: case 12:
			codes[which].replaceValue =val;
			break;
	}
}

int	MAPINT	readCHR_bios (int bank, int addr) {
	if (addr &0x04)
		return (addr &0x10? 0xF0: 0x00) | (addr &0x20? 0x0F: 0x00);
	else
		return (addr &0x40? 0xF0: 0x00) | (addr &0x80? 0x0F: 0x00);
}
int	MAPINT	readNT_bios (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr &0x3FF];
}
void	MAPINT	writeNT_bios (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[bank &1][addr &0x3FF] =val;
}

void	sync() {
	if (gameMode) {
		for (int bank =0x0; bank <0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, passPPUWrite);
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, cpuRead_game);
			SetCPUReadHandlerDebug(bank, cpuReadDebug_game);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
	} else {
		for (int bank =0x0; bank <0x8; bank++) {
			SetPPUReadHandler(bank, readCHR_bios);
			SetPPUReadHandlerDebug(bank, readCHR_bios);
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, cpuRead_bios);
			SetCPUReadHandlerDebug(bank, cpuRead_bios);
			SetCPUWriteHandler(bank, cpuWrite_bios);

			SetPPUReadHandler(bank, readNT_bios);
			SetPPUReadHandlerDebug(bank, readNT_bios);
			SetPPUWriteHandler(bank, writeNT_bios);
		}
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\GG.BIN"), bios, sizeof(bios));
	Description =_T("Galoob Game Genie");
	
	if (adMI->Load) adMI->Load();	
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->CPUCycle) MI->CPUCycle =adMI->CPUCycle;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	gameMode =false;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) {
		gameMode =false;
		for (auto& code: codes) {
			code.enable =false;
			code.compare =false;
			code.addr =0xFFFF;
			code.compareValue =0x00;
			code.replaceValue =0x00;
		}
	}
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, gameMode);
	for (auto& code: codes) {
		SAVELOAD_BOOL(stateMode, offset, data, code.enable);
		SAVELOAD_BOOL(stateMode, offset, data, code.compare);
		SAVELOAD_WORD(stateMode, offset, data, code.addr);
		SAVELOAD_BYTE(stateMode, offset, data, code.compareValue);
		SAVELOAD_BYTE(stateMode, offset, data, code.replaceValue);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_GAME_GENIE;
} // namespace GameGenie

MapperInfo plugThruDevice_GameGenie ={
	&GameGenie::deviceNum,
	_T("Galoob Game Genie"),
	COMPAT_FULL,
	GameGenie::load,
	GameGenie::reset,
	NULL,
	NULL,
	NULL,
	GameGenie::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice