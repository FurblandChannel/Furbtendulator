#include	"..\DLL\d_iNES.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg[4];
bool		enableIRQ;
bool		buttonState;
int		count;

#define		mirrorH !!(reg[0] &1)
#define		unrom   !!(reg[0] &2)
#define		nrom128 !!(reg[0] &4)
#define		prg      ((reg[1] &7) | (reg[2] <<3))
#define		extBank    reg[3]

void	sync (void) {
	if (ROM->INES_MapperNum ==551) {
		EMU->SetPRG_RAM8(0x6, 0);
		EMU->SetCHR_ROM8(0x0, extBank);
		iNES_SetMirroring();
	} else {
		EMU->SetPRG_RAM8(0x6, extBank);
		EMU->SetCHR_RAM8(0x0, 0);
		if (mirrorH)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
	EMU->SetPRG_ROM16(0x8, prg &~(!nrom128*!unrom  ));
	EMU->SetPRG_ROM16(0xC, prg |  !nrom128 |unrom*6);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x800) {
		reg[addr &3] =val;
		sync();
	}
}

int	MAPINT	readInfrared (int bank, int addr) {
	EMU->SetIRQ(1);
	return 0;
}

void	MAPINT	cpuCycle () {
	if (enableIRQ) {
		bool newButtonState =!!GetKeyState(VK_LBUTTON);
		if (!buttonState && newButtonState) EMU->SetIRQ(0);
		buttonState =newButtonState;
	}
}

void	MAPINT	writeInfrared (int bank, int addr, int val) {
	enableIRQ =!!(val &0x80);
	if (!enableIRQ) EMU->SetIRQ(1);
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_MapperNum ==178) MapperInfo_178.CPUCycle =ROM->INES2_SubMapper ==1? cpuCycle: NULL;
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	sync();
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);
	
	if (ROM->INES2_SubMapper==1) {
		EMU->SetCPUReadHandler(0x5, readInfrared);
		EMU->SetCPUWriteHandler(0x6, writeInfrared);
		enableIRQ =true;
		buttonState =false;
		count =0;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (ROM->INES2_SubMapper==1) {
		SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
		SAVELOAD_BOOL(stateMode, offset, data, buttonState);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum178 =178;
uint16_t mapperNum551 =551;
} // namespace

MapperInfo MapperInfo_178 ={
	&mapperNum178,
	_T("外星 FS305/南晶 NJ0430/PB030703-1x1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_551 ={
	&mapperNum551,
	_T("晶科泰 KT-xxx"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};