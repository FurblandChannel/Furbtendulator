#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
bool		mapCIRAM;

void	sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0x1FF, 0x000);
	EMU->SetPRG_ROM8(0x6, 0x0F);

	VRC24::syncMirror();
	if (mapCIRAM) EMU->SetCHR_NT1(0x3, 1);
}

void	MAPINT	writeCIRAMSwitch (int bank, int addr, int val) {
	if (addr &0x800) {
		mapCIRAM =!!(bank &1);
		sync();
	} else
		VRC24::writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0xD, writeCIRAMSwitch);
	EMU->SetCPUWriteHandler(0xE, writeCIRAMSwitch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, mapCIRAM);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =542;
} // namespace

MapperInfo MapperInfo_542 = {
	&mapperNum,
	_T("毛泽东诞辰: 一百周年纪念 1893-1993"),
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