#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	reset (RESET_TYPE resetType) {
	EMU->SetPRG_ROM32(0x6, 0);
	EMU->SetPRG_ROM8 (0xE, 3);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

uint16_t mapperNum =767;
} // namespace

MapperInfo MapperInfo_767 ={
	&mapperNum,
	_T("FDS Conversion Test"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};