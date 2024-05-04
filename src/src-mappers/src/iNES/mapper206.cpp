#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	if (ROM->INES2_SubMapper ==1)
		EMU->SetPRG_ROM32(0x8, 0x00);
	else
		N118::syncPRG(0x0F, 0x00);
	N118::syncCHR(0x3F, 0x00);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	if (CART_VRAM)
		MapperInfo_206.Description = _T("Nintendo DRROM/Tengen 800004");
	else
		MapperInfo_206.Description = _T("Nintendo DEROM/DE1ROM/Tengen MIMIC-1/Namco N118");
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =206;
} // namespace

MapperInfo MapperInfo_206 ={
	&mapperNum,
	_T("Namco N118"),
	COMPAT_FULL,
	load,
	N118::reset,
	NULL,
	NULL,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};