#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_JV001.h"

namespace {
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, JV001::output >>4);
	EMU->SetCHR_ROM8(0x0, JV001::output);
}

int	MAPINT	readChip (int bank, int addr) {
	if (bank ==0x4 && addr <=0x20)
		return readAPU(bank, addr);
	else
		return JV001::read(bank, addr) &0x3F | *EMU->OpenBus &~0x3F;
}

void	MAPINT	writeChip (int bank, int addr, int val) {
	if (bank ==0x4 && addr <=0x20) writeAPU(bank, addr, val);
	JV001::write(bank, addr, val &0x3F);
}

BOOL	MAPINT	load (void) {
	JV001::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	JV001::reset(resetType);
	
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int i =0x4; i<=0x5; i++) EMU->SetCPUReadHandler(i, readChip);
	for (int i =0x4; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeChip);
}

uint16_t mapperNum =136;
} // namespace

MapperInfo MapperInfo_136 ={
	&mapperNum,
	_T("聖謙 3011/SA-002"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	JV001::saveLoad,
	NULL,
	NULL
};
