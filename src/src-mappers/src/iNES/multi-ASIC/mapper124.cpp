#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_Latch.h"

#define MAPPER_UNROM 0
#define MAPPER_AMROM 1
#define MAPPER_MMC1  2
#define MAPPER_MMC3  3

#define mapper  (mode[0] >>4 &3)
#define prgOR   (mode[1] <<4 &0x1F0)
#define chrOR   (mode[0] <<7 &0x780)
#define rom6  !!(mode[1] &0x20)
#define rom8   !(mode[1] &0x80)
#define chrram !(mode[1] &0x40)

namespace {
uint8_t		audioEnable;
uint8_t		ram[4096];
uint8_t		mode[2];
FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPUWrite	writeAPU;

void	sync (void) {
	switch (mapper) {
		case MAPPER_UNROM:
			EMU->SetPRG_ROM16(0x8, prgOR >>1 | Latch::data &7);
			EMU->SetPRG_ROM16(0xC, prgOR >>1 |              7);
			EMU->Mirror_V();
			break;
		case MAPPER_AMROM: 
			EMU->SetPRG_ROM32(0x8, prgOR >>2 | Latch::data &7);
			if (Latch::data &0x10)
				EMU->Mirror_S1();
			else	
				EMU->Mirror_S0();
			break;
		case MAPPER_MMC1: 
			MMC1::syncWRAM(0);
			MMC1::syncPRG(0x07, prgOR >>1);
			MMC1::syncCHR(0x1F, chrOR >>2);
			MMC1::syncMirror();
			break;
		case MAPPER_MMC3: 
			MMC3::syncWRAM();
			MMC3::syncPRG(mode[1] &0x20? 0x0F: 0x1F, prgOR);
			MMC3::syncCHR(mode[0] &0x40? 0x7F: 0xFF, chrOR);
			MMC3::syncMirror();
			break;
	}
	EMU->SetPRG_ROM4(0x5, 0x380 +0x5);	
	if (rom6)   EMU->SetPRG_ROM8 (0x6, (0x380 +0x6) >>1);
	if (rom8)   EMU->SetPRG_ROM32(0x8, (0x380 +0x8) >>3);
	if (chrram) EMU->SetCHR_RAM8 (0x0, 0x0);
}

void	applyMode (void) {
	switch (mapper) {
		case MAPPER_UNROM:
		case MAPPER_AMROM:
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, Latch::write);
			break;
		case MAPPER_MMC1:
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
			break;
		case MAPPER_MMC3:
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC3::write);
			break;
	}
}

int	MAPINT	readCoinDIP (int bank, int addr) {
	if ((addr &0xF0F) ==0xF0F) {
		return (GetKeyState('C')? 0x80: 0x00) | ROM->dipValue;
	} else
		return readAPU(bank, addr);
}

int	MAPINT	readCoinDIPDebug (int bank, int addr) {
	if ((addr &0xF0F) ==0xF0F) {
		return (GetKeyState('C')? 0x80: 0x00) | ROM->dipValue;
	} else
		return readAPUDebug(bank, addr);
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (addr &0x10)
		audioEnable =val;
	else {
		mode[addr &1] =val;
		applyMode();
		sync();
	}
}

int	MAPINT	readRAM (int bank, int addr) {
	return ram[addr];
}

void	MAPINT	writeRAM (int bank, int addr, int val) {
	ram[addr] =val;
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC1::load(sync, MMC1Type::MMC1A);
	MMC3::load(sync);
	Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType){
	if (resetType ==RESET_HARD) {
		for (auto& c: mode) c =0;
		applyMode();
	}
	sync();

	MMC1::reset(resetType);
	MMC3::reset(resetType);
	Latch::reset(resetType);
	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	writeAPU     =EMU->GetCPUWriteHandler    (0x4);
	EMU->SetCPUReadHandler(0x4, readCoinDIP);
	EMU->SetCPUReadHandlerDebug(0x4, readCoinDIPDebug);
	EMU->SetCPUWriteHandler(0x5, writeASIC);
	
	EMU->SetCPUReadHandler     (0x0, readRAM);
	EMU->SetCPUReadHandlerDebug(0x0, readRAM);
	EMU->SetCPUWriteHandler    (0x0, writeRAM);
}

void	MAPINT	cpuCycle (void) {
	if (mapper ==MAPPER_MMC1) MMC1::cpuCycle();
	if (mapper ==MAPPER_MMC3) MMC3::cpuCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (mapper ==MAPPER_MMC3) MMC3::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	for (auto& c: mode) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) {
		applyMode();
		sync();
	}
	return offset;
}

uint16_t mapperNum =124;
} // namespace

MapperInfo MapperInfo_124 ={
	&mapperNum,
	_T("Super Game Mega Type 3"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};