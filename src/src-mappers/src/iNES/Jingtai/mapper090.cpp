#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
void	sync (void) {
	JY::syncPRG(0x3F, JY::outerBank <<5 &~0x3F);
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
	JY::load(sync, ROM->INES_MapperNum !=90);
	return TRUE;
}

uint16_t mapperNum035 =35;
uint16_t mapperNum090 =90;
uint16_t mapperNum209 =209;
uint16_t mapperNum211 =211;
} // namespace

MapperInfo MapperInfo_035 = {
	&mapperNum035,
	_T("晶太 EL870914C"),
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
MapperInfo MapperInfo_090 = {
	&mapperNum090,
	_T("晶太 EL861226C"),
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
MapperInfo MapperInfo_209 = {
	&mapperNum209,
	_T("晶太 YY850629C"),
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
MapperInfo MapperInfo_211 = {
	&mapperNum211,
	_T("晶太 EL860339C"),
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