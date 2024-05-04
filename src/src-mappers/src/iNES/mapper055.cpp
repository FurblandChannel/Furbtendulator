#include	"..\DLL\d_iNES.h"

namespace {
void	MAPINT	reset (RESET_TYPE resetType) {
	EMU->SetPRG_ROM4 (0x6, 8);
	EMU->SetPRG_RAM4 (0x7, 0);
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8 (0x0, 0);
	iNES_SetMirroring();
}

uint16_t mapperNum =55;
} // namespace

MapperInfo MapperInfo_055 ={
	&mapperNum,
	_T("NCN-35A"),
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