#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_JV001.h"

namespace {
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, JV001::output >>4 &2 | JV001::output &1);
	EMU->SetCHR_ROM8(0x0, JV001::output >>1 &0xF);
}

int	MAPINT	readChip (int bank, int addr) {
	if (bank ==0x4 && addr <=0x20)
		return readAPU(bank, addr);
	else {
		int val =JV001::read(bank, addr);
		return val <<2 &0xFC | val >>6 &0x03;
	}
}

void	MAPINT	writeChip (int bank, int addr, int val) {
	if (bank ==0x4 && addr <=0x20) writeAPU(bank, addr, val);
	JV001::write(bank, addr, val >>2 &0x3F | val <<6 &0xC0);
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

uint16_t mapperNum =147;
} // namespace

MapperInfo MapperInfo_147 ={
	&mapperNum,
	_T("聖謙 3018"),
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
