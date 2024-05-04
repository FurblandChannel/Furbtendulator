#include "..\DLL\d_iNES.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		nt[4];
bool		enableIRQ;
int		counter;

void	sync (void) {
	EMU->SetPRG_ROM8(0x8, prg[0]);
	EMU->SetPRG_ROM8(0xA, prg[1]);
	EMU->SetPRG_ROM8(0xC, prg[2]);
	EMU->SetPRG_ROM8(0xE,   0xFF);
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	for (int bank =0; bank <4; bank++) EMU->SetCHR_NT1 (bank |8, nt[bank &1]);
}	
void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr >>4 &7) {
		case 0:	prg[addr &3] =val;
			break;
		case 1:	chr[addr &3 |0] =val;
			if (ROM->INES2_SubMapper ==1) nt[addr &3 |0] =val >>7;
			break;
		case 2:	chr[addr &3 |4] =val;
			break;
		case 3:	enableIRQ =true;
			counter =0;
			break;
		case 4:	enableIRQ =false;
			break;
		case 5:	if (ROM->INES2_SubMapper ==0) nt[addr &3] =val;
			break;
	}
	sync();
}

void	MAPINT	reset (RESET_TYPE ResetType) {	
	if (ResetType == RESET_HARD) {
		for (auto& c: prg) c =0;
		for (auto& c: chr) c =0;
		for (auto& c: nt ) c =0;		
		enableIRQ =false;
		counter =0;
		EMU->SetIRQ(1);
	}
	sync();
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: nt ) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_LONG(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

void	MAPINT	cpuCycle() {
	EMU->SetIRQ(++counter &(ROM->INES2_SubMapper ==1? 0x1000: 0x0400) && enableIRQ? 0: 1);
}

uint16_t mapperNum =417;
} // namespace

MapperInfo MapperInfo_417 = {
	&mapperNum,
	_T("Fine Studio Happy New Years 1990"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
