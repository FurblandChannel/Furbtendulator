#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		mode;
uint8_t		latch;

void	sync (void) {
	if (mode &1) {
		MMC1::syncPRG(0x07, 0x08);
		MMC1::syncCHR_ROM(0x1F, 0x20);
		MMC1::syncMirror();
	} else {
		EMU->SetPRG_ROM16(0x8, ((mode &2) <<1) | ((latch >>4) &3));
		EMU->SetPRG_ROM16(0xC, ((mode &2) <<1) |               3 );
		EMU->SetCHR_ROM8(0, latch &0xF);
		EMU->Mirror_V();
	}
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		mode =val;
		sync();
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (mode &1)
		MMC1::write(bank, addr, val);
	else {
		latch =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1A);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		mode =0;
		latch =0;
	}
	MMC1::reset(resetType);
	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank<=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeMode);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =297;
} // namespace

MapperInfo MapperInfo_297 ={
	&mapperNum,
	_T("TXC 01-22110-000"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};