#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
void	sync (void) {
	if (JY::outerBank &0x04)
		JY::syncPRG(0x3F, JY::outerBank <<4 &~0x3F);
	else
		JY::syncPRG(0x1F, JY::outerBank <<4 &~0x1F);
	JY::syncCHR(0x1FF, JY::outerBank <<8 &0x300);
	JY::syncNT (0x1FF, JY::outerBank <<8 &0x300);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	JY::load(sync, true);
	return TRUE;
}

uint16_t mapperNum =421;
} // namespace

MapperInfo MapperInfo_421 = {
	&mapperNum,
	_T("晶太 SC871115C"),
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
