#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t		prg6000;

void	sync (void) {
	VRC24::syncPRG(0x3F, 0x00);
	VRC24::syncCHR(0xFFF, 0x000);
	VRC24::syncMirror();
	EMU->SetPRG_ROM8(0x6, prg6000);
}

void	MAPINT	write6000 (int bank, int addr, int val) {
	prg6000 =addr &0x3F;
	sync();
}

BOOL	MAPINT	load (void) {
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	VRC24::reset(resetType);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, write6000);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg6000);
	offset =VRC24::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =183;
} // namespace

MapperInfo MapperInfo_183 = {
	&MapperNum,
	_T("09035"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};