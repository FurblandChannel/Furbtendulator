#include	"..\DLL\d_iNES.h"
#include	<vector>
#include	<queue>
#include	"..\Hardware\simplefdc.hpp"

namespace {
uint8_t		reg5000;
uint8_t		reg5200;
uint8_t		mode;
FPPURead	readNT;
FPPURead	readCHR;
FDC		fdc;

uint8_t		dacStatus;
uint8_t		dacOutput;
int		dacCount;

uint8_t		chrBank;
int		lastNTAddr;
bool		inSprite;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 128/8);
	
	if (reg5000 &0x80) {
		if (reg5200 &0x04)
			EMU->SetPRG_RAM32(0x8, reg5000 &3);
		else {
			EMU->SetPRG_RAM16(0x8, reg5000 &7);
			EMU->SetPRG_ROM16(0xC,          0);
		}
	} else {
		if (reg5200 &0x04)
			EMU->SetPRG_ROM32(0x8, reg5000);
		else {
			EMU->SetPRG_ROM16(0x8, reg5000);
			EMU->SetPRG_ROM16(0xC,       0);
		}
	}
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (reg5200 &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	interceptNTRead (int bank, int addr) {
	inSprite =addr ==lastNTAddr;
	lastNTAddr =addr;
	
	chrBank =bank >>(reg5200 &0x01) &0x01? 4: 0;
	return readNT(bank, addr);
}

int 	MAPINT	interceptCHRRead (int bank, int addr) {
	if (reg5200 &0x02 && ~reg5200 &0x07 && !inSprite)
		return readCHR(bank &3 | chrBank, addr);
	else
		return readCHR(bank, addr);
}

int	MAPINT	read5 (int bank, int addr) {
	switch (addr &0xF00) {
		case 0x300:
			return dacStatus; break;
		case 0x600:
			return fdc.readIO(addr);
		default:
			return *EMU->OpenBus;
	}
}

void	MAPINT	write5 (int bank, int addr, int val) {
	switch (addr &0xF00) {
		case 0x000:
			reg5000 =val;
			sync();
			break;
		case 0x200:
			reg5200 =val;
			sync();
			break;
		case 0x300:
			dacOutput =val;
			dacStatus =0x00;
			break;
		case 0x500:
			if (addr ==0x501) addr =2; else
			if (addr ==0x500) addr =7;
			fdc.writeIO(addr, val);
			break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg5000 =0;
	reg5200 =0;
	sync();
	fdc.reset();
	(EMU->GetCPUWriteHandler(0x4))(0x4, 0x017, 0x40); // FrameIRQ interferes with FDC IRQ

	readCHR =EMU->GetPPUReadHandler(0x0);
	readNT  =EMU->GetPPUReadHandler(0x8);
	for (int bank =8; bank <16; bank++) EMU->SetPPUReadHandler     (bank, interceptNTRead);
	for (int bank =8; bank <16; bank++) EMU->SetPPUReadHandlerDebug(bank, readNT);
	for (int bank =0; bank < 8; bank++) EMU->SetPPUReadHandler     (bank, interceptCHRRead);
	for (int bank =0; bank < 8; bank++) EMU->SetPPUReadHandlerDebug(bank, interceptCHRRead);
	
	EMU->SetCPUReadHandler(0x5, read5);
	EMU->SetCPUWriteHandler(0x5, write5);
}

void	MAPINT	cpuCycle (void) {
	if (ROM->changedDisk35) {
		fdc.ejectDisk(0);
		if (ROM->diskData35 !=NULL) fdc.insertDisk(0, &(*ROM->diskData35)[0], ROM->diskData35->size(), true);
	}
	fdc.run();
	if (fdc.irqRaised())
		EMU->SetIRQ(0);
	else
		EMU->SetIRQ(1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg5000);
	SAVELOAD_BYTE(stateMode, offset, data, reg5200);
	if (mode ==STATE_LOAD)	sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	if (++dacCount ==1789772 /11025) {
		dacCount =0;
		dacStatus =0x80 | (dacOutput &0x7F);
	}
	return dacOutput *16;
}

uint16_t mapperNum =518;
} // namespace

MapperInfo MapperInfo_518 = {
	&mapperNum,
	_T("Subor SB-97"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSound,
	NULL
};

/*
	480F	Printer control?
	
*/