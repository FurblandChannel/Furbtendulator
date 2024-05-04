#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	sync() {
	EMU->SetPRG_ROM8(0x6, Latch::addr >>2 | (Latch::addr &0x20? 0x04: 0x00));
	EMU->SetPRG_ROM32(0x8, 0x2);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	FDSsound::ResetBootleg(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =549;
} // namespace

MapperInfo MapperInfo_549 ={
	&MapperNum,
	_T("Kaiser KS-7016B"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};