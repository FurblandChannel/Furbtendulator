#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
void	sync (void) {
	JY::syncPRG(0x1F, JY::outerBank <<4 &~0x1F);
	if (JY::outerBank &0x20) {
		JY::syncCHR(0x1FF,                            JY::outerBank <<6 &0x600);
		JY::syncNT (0x1FF,                            JY::outerBank <<6 &0x600);
	} else {
		JY::syncCHR(0x0FF, JY::outerBank <<8 &0x100 | JY::outerBank <<6 &0x600);
		JY::syncNT (0x0FF, JY::outerBank <<8 &0x100 | JY::outerBank <<6 &0x600);
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	JY::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =282;
} // namespace

MapperInfo MapperInfo_282 = {
	&mapperNum,
	_T("晶太 860224C"),
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