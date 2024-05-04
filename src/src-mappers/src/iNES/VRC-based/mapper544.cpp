#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
FPPUWrite	writeCHR;
uint8_t		maskCHRBank;
uint8_t		maskCompare;
uint8_t		nt[4];
uint8_t		cpuC;

void	sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	EMU->SetPRG_ROM8(0xC, cpuC);
	
	for (int bank =0x0; bank <0x8; bank++) {
		int val =VRC24::chr[bank];
		if ((val &maskCHRBank) ==maskCompare)
			EMU->SetCHR_RAM1(bank, val);
		else
			EMU->SetCHR_ROM1(bank, val);
	}
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCHR_NT1(bank, nt[bank &3]);
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	if (addr &4)
		nt[addr &3] =val;
	else
		cpuC =val;
	sync();
}

static const uint8_t compareMasks[8] = {
	0x28, 0x00, 0x4C, 0x64, 0x46, 0x7C, 0x04, 0xFF
};
void	MAPINT	interceptCHRWrite (int bank, int addr, int val) {
	uint8_t chr =VRC24::chr[bank];
	if (chr &0x80) {
		maskCHRBank =chr &0x10? 0x00: chr &0x40? 0xFE: 0xFC;
		maskCompare =chr &0x10? 0xFF: compareMasks[chr >>1 &1 | chr >>2 &2 | chr >>4 &4];
		sync();
	}
	writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x400, 0x800, writeExtra, true, 1);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		maskCHRBank =0xFC;
		maskCompare =0x28;
		nt[0] =0xE0;
		nt[1] =0xE0;
		nt[2] =0xE1;
		nt[3] =0xE1;
		cpuC =0xFE;
	}
	VRC24::reset(resetType);
	writeCHR =EMU->GetPPUWriteHandler(0x0);
	for (int bank =0x0; bank<=0x7; bank++) EMU->SetPPUWriteHandler(bank, interceptCHRWrite);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, maskCHRBank);
	SAVELOAD_BYTE(stateMode, offset, data, maskCompare);
	SAVELOAD_BYTE(stateMode, offset, data, cpuC);
	for (auto& c: nt) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =544;
} // namespace

MapperInfo MapperInfo_544 ={
	&mapperNum,
	_T("外星 FS306"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};