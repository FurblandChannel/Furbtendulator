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
namespace VenusMiniDoctor {
union {
	uint8_t k8 [4][8][1024];
	uint8_t b  [32*1024];
} chrram;

bool	enabled;
uint8_t	chr;
void	sync();

// Loading and playing state
void	MAPINT	cpuWrite_reg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	switch ((addr >>8) &3) {
		case 1:	enabled =true;
			sync();
			break;
		case 3:	enabled =false;
			chr =val &3;
			sync();
			break;
	}
}

int	MAPINT	cpuRead (int bank, int addr) {
	if ((bank &~1) ==0x6)
		return *EI.OpenBus;
	else {
		if (bank >=0xE) bank &=0x7;
		return passCPURead(bank, addr);
	}
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if ((bank &~1) ==0x6)
		return *EI.OpenBus;
	else {
		if (bank >=0xE) bank &=0x7;
		return passCPUReadDebug(bank, addr);
	}
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	chr =val &3;
}
int	MAPINT	ppuRead_chr (int bank, int addr) {
	if (chr)
		return chrram.k8[chr][bank][addr];
	else
		return passPPURead(bank, addr);
}
void	MAPINT	ppuWrite_chr (int bank, int addr, int val) {
	if (chr) {
		chrram.k8[chr][bank][addr] =val;
		if (!enabled) chr =0;
	} else
		passPPUWrite(bank, addr, val);
}

void	sync() {
	SetCPUWriteHandler(0x4, cpuWrite_reg);
	SetCPUWriteHandler(0x5, cpuWrite_reg);
	for (int bank =0x6; bank<=0xF; bank++) {
		SetCPUReadHandler     (bank, enabled? cpuRead:      passCPURead);
		SetCPUReadHandlerDebug(bank, enabled? cpuReadDebug: passCPUReadDebug);
		SetCPUWriteHandler    (bank, enabled? cpuWrite:     passCPUWrite);
	}
	for (int bank =0x0; bank<=0x7; bank++) {
		SetPPUReadHandler     (bank, ppuRead_chr);
		SetPPUReadHandlerDebug(bank, ppuRead_chr);
		SetPPUWriteHandler    (bank, ppuWrite_chr);
	}
	for (int bank =0x8; bank<=0xF; bank++) {
		SetPPUReadHandler     (bank, passPPURead);
		SetPPUReadHandlerDebug(bank, passPPUReadDebug);
		SetPPUWriteHandler    (bank, passPPUWrite);
	}
}

BOOL	MAPINT	load (void) {
	Description =_T("Venus Mini Doctor");
	
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
		for (auto& c: chrram.b) c =0x00;
		enabled =false;
		chr =0;
	}	
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);	
	SAVELOAD_BOOL(stateMode, offset, data, enabled);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_VENUS_MD;
} // namespace VenusMiniDoctor

MapperInfo plugThruDevice_MiniDoctor ={
	&VenusMiniDoctor::deviceNum,
	_T("Venus Mini Doctor"),
	COMPAT_FULL,
	VenusMiniDoctor::load,
	VenusMiniDoctor::reset,
	NULL,
	NULL,
	NULL,
	VenusMiniDoctor::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice