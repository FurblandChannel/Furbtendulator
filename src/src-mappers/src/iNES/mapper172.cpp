#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_JV001.h"

namespace {
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0x0, JV001::output);
	if (JV001::X)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();

}

int	MAPINT	readChip (int bank, int addr) {
	if (bank ==0x4 && addr <=0x20)
		return readAPU(bank, addr);
	else {
		int val =JV001::read(bank, addr);
		return val<<5&0x20 | val<<3&0x10 | val<<1&0x08 | val>>1&0x04 | val>>3&0x02 | val>>5&0x01 | *EMU->OpenBus&0xC0;
	}
}

void	MAPINT	writeChip (int bank, int addr, int val) {
	if (bank ==0x4 && addr <=0x20) writeAPU(bank, addr, val);
	JV001::write(bank, addr, val<<5&0x20 | val<<3&0x10 | val<<1&0x08 | val>>1&0x04 | val>>3&0x02 | val>>5&0x01);
}

BOOL	MAPINT	load (void) {
	JV001::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	JV001::reset(resetType);
	
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int i =0x4; i<=0x5; i++) EMU->SetCPUReadHandler(i, readChip);
	for (int i =0x4; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeChip);
}

uint16_t mapperNum =172;
} // namespace

MapperInfo MapperInfo_172 ={
	&mapperNum,
	_T("Super Mega SMCYII-900"),
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
