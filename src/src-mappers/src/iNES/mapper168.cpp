#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg;
uint16_t	counter;
bool		disableIRQ;
bool		protectCHR;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, reg >>6);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	if (ROM->INES2_CHRRAM >=0xA0) {	// If all of CHR-RAM is battery-backed (64 KiB)
		if (protectCHR)
			for (int bank =0; bank <8; bank++) EMU->SetCHR_OB1(bank);
		else {
			EMU->SetCHR_RAM4(0x0, 0);
			EMU->SetCHR_RAM4(0x4, reg &0xF ^8);
		}
	} else { // Only the second half of CHR-RAM is battery-backed (32 KiB)
		EMU->SetCHR_RAM4(0x0, 0);
		if (protectCHR && reg &8)
			for (int bank =0; bank <8; bank++) EMU->SetCHR_OB1(bank);
		else
			EMU->SetCHR_RAM4(0x4, reg &0xF ^8);
	}
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	bool newDisableIRQ =!!(addr &0x80);
	if (disableIRQ && !newDisableIRQ) {
		protectCHR =false;
		sync();
	}
	disableIRQ =newDisableIRQ;
	if (disableIRQ) EMU->SetIRQ(1);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		reg =0;
		counter =0;
		disableIRQ =false;
		protectCHR =true;
	}
	sync();	
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIRQ);
}

void	MAPINT	cpuCycle (void) {
	if (disableIRQ)
		counter =0;
	else
		EMU->SetIRQ(++counter &1024? 0: 1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BOOL(stateMode, offset, data, disableIRQ);
	SAVELOAD_BOOL(stateMode, offset, data, protectCHR);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =168;
} // namespace

MapperInfo MapperInfo_168 ={
	&mapperNum,
	_T("Racermate Challenge 2"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};

