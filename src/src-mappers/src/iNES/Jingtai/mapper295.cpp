#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
FCPURead readAPU;
FCPUWrite writeAPU;

void	sync (void) {
	JY::syncPRG(0x0F, JY::outerBank <<4);
	JY::syncCHR(0x7F, JY::outerBank <<7);
	JY::syncNT (0x7F, JY::outerBank <<7);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	JY::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	JY::reset(resetType);
	FDSsound::ResetBootleg(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =JY::saveLoad(stateMode, offset, data);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =295;
} // namespace

MapperInfo MapperInfo_295 = {
	&mapperNum,
	_T("晶太 YY860216C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	JY::cpuCycle,
	JY::ppuCycle,
	JY::saveLoad,
	FDSsound::GetBootleg,
	NULL
};