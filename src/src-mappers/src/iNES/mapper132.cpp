#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_TXCLatch.h"

namespace {
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, TXCLatch::output >>2 &1);
	EMU->SetCHR_ROM8(0x0, TXCLatch::output &3);
}

int	MAPINT	readChip (int bank, int addr) {
	if (bank ==0x4 && addr <=0x20)
		return readAPU(bank, addr);
	else
		return TXCLatch::read(bank, addr) &0x0F | *EMU->OpenBus &~0x0F;
}

void	MAPINT	writeChip (int bank, int addr, int val) {
	if (bank ==0x4 && addr <=0x20) writeAPU(bank, addr, val);
	TXCLatch::write(bank, addr, val &0x0F);
}

BOOL	MAPINT	load (void) {
	TXCLatch::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	TXCLatch::reset(resetType);
	
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int i =0x4; i<=0x5; i++) EMU->SetCPUReadHandler(i, readChip);
	for (int i =0x4; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeChip);
}

uint16_t mapperNum =132;
} // namespace

MapperInfo MapperInfo_132 ={
	&mapperNum,
	_T("TXC 01-22003-400/01-22111-100/01-22270-000"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	TXCLatch::saveLoad,
	NULL,
	NULL
};