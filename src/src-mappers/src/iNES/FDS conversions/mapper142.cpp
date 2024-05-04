#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_KS202.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	sync (void) {
	KS202::syncPRG(0x0F, 0x00);
	EMU->SetCHR_RAM8(0x0, 0);
}

BOOL	MAPINT	load (void) {
	KS202::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	FDSsound::ResetBootleg(resetType);
	KS202::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =KS202::saveLoad(stateMode, offset, data);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =142;
} // namespace

MapperInfo MapperInfo_142 ={
	&mapperNum,
	_T("Kaiser KS-7032"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	KS202::cpuCycle,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};