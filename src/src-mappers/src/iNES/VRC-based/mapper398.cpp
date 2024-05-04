#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {	
uint8_t		currentCHRBank;
uint8_t		latch;
FPPURead	readCHR;

void	sync (void) {
	if (latch &0x80) {
		EMU->SetPRG_ROM32(0x8,        latch >>5 &6 | VRC24::chr[currentCHRBank] >>2 &1);
		EMU->SetCHR_ROM8 (0x0, 0x40 | latch >>3 &8 | VRC24::chr[currentCHRBank]     &7);
	} else {
		VRC24::syncPRG(0x0F, 0x00);
		VRC24::syncCHR(0x1FF, 0x00);
	}
	VRC24::syncMirror();
}

int	MAPINT	trapCHRRead (int bank, int addr) {
	currentCHRBank =bank;
	sync();
	return readCHR(bank, addr);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	latch =addr &0xFF;
	sync();
	VRC24::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	latch =0xC0;
	VRC24::reset(resetType);
	
	readCHR =EMU->GetPPUReadHandler(0x0);
	for (int bank =0; bank <8; bank++) {
		EMU->SetPPUReadHandler(bank, trapCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, readCHR);
	}
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =398;
} // namespace

MapperInfo MapperInfo_398 = {
	&mapperNum,
	_T("晶太 YY840820C"),
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