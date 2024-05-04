#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	sync (void) {
	JY::syncPRG(0x1F, JY::outerBank <<4 &~0x1F);
	JY::syncCHR(0x7F, JY::outerBank <<7);
	JY::syncNT (0x7F, JY::outerBank <<7);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	JY::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =397;
} // namespace

MapperInfo MapperInfo_397 = {
	&mapperNum,
	_T("晶太 YY850439C"),
	COMPAT_FULL,
	load,
	JY::reset,
	NULL,
	JY::cpuCycle,
	JY::ppuCycle,
	JY::saveLoad,
	NULL,
	NULL
};