#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (ROM->INES2_SubMapper ==1)
		EMU->SetPRG_ROM32(0x8, Latch::data);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::data);
	}
	EMU->SetCHR_ROM8(0x0, Latch::data);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	if (resetType ==RESET_HARD) Latch::data =0xFF;
	sync();
}

uint16_t mapperNum =343;
} // namespace

MapperInfo MapperInfo_343 ={
	&mapperNum,
	_T("21-in-1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};