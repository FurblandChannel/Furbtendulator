#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>4 &7);
	iNES_SetCHR_Auto8(0x0, Latch::data &0xF);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_PRGSize ==128/16 && ROM->INES_CHRSize ==32/8)
		MapperInfo_066.Description = _T("Nintendo GNROM");
	else if (ROM->INES_PRGSize ==64/16 && ROM->INES_CHRSize <=32/8)
		MapperInfo_066.Description = _T("Nintendo MHROM");
	else
		MapperInfo_066.Description = _T("GNROM/MHROM (nonstandard)");
	Latch::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =66;
} // namespace

MapperInfo MapperInfo_066 = {
	&mapperNum,
	_T("Nintendo GNROM/MHROM"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
