#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
void	sync (void) {
	JY::syncPRG(0x1F, JY::outerBank <<5);
	JY::syncCHR(0xFF, JY::outerBank <<8);
	JY::syncNT (0xFF, JY::outerBank <<8);
}

BOOL	MAPINT	load (void) {
	JY::load(sync, true);
	iNES_SetSRAM();
	return TRUE;
}

uint16_t mapperNum =281;
} // namespace

MapperInfo MapperInfo_281 = {
	&mapperNum,
	_T("晶太 YY860417C"),
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