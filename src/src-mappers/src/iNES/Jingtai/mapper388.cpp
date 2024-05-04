#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
void	sync (void) {
	JY::syncPRG(0x1F, JY::outerBank <<3 &0x60);
	if (JY::outerBank &0x20) {
		JY::syncCHR(0x1FF,                            JY::outerBank <<8 &0x200);
		JY::syncNT (0x1FF,                            JY::outerBank <<8 &0x200);
	} else {
		JY::syncCHR(0x0FF, JY::outerBank <<8 &0x100 | JY::outerBank <<8 &0x200);
		JY::syncNT (0x0FF, JY::outerBank <<8 &0x100 | JY::outerBank <<8 &0x200);
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	JY::load(sync, false);
	return TRUE;
}

uint16_t mapperNum =388;
} // namespace

MapperInfo MapperInfo_388 = {
	&mapperNum,
	_T("晶太 YY850835C"),
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