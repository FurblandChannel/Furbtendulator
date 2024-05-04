#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_TXCLatch.h"

namespace {
uint8_t		chr;
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, TXCLatch::output &3);
	EMU->SetCHR_ROM8(0x0, chr);
	iNES_SetMirroring();
}

int	MAPINT	readChip (int bank, int addr) {
	if (bank ==0x4 && addr <=0x20)
		return readAPU(bank, addr);
	else
		return TXCLatch::read(bank, addr) <<4 &0x30 | *EMU->OpenBus &~0x30;
}

void	MAPINT	writeChip (int bank, int addr, int val) {
	if (bank ==0x4 && addr <=0x20) writeAPU(bank, addr, val);
	if (bank ==0x4 && addr &0x200) chr =val;
	TXCLatch::write(bank, addr, val >>4 &3);
}

BOOL	MAPINT	load (void) {
	TXCLatch::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	TXCLatch::reset(resetType);
	
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int i =0x4; i<=0x5; i++) EMU->SetCPUReadHandler(i, readChip);
	for (int i =0x4; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeChip);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =TXCLatch::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =36;
} // namespace

MapperInfo MapperInfo_036 ={
	&mapperNum,
	_T("TXC 01-22000-200/400"),
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