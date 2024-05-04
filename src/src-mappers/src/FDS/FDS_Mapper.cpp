#include	"..\DLL\d_FDS.h"
#include	"..\Hardware\h_FDS.h"

namespace {
} // namespace

MapperInfo MapperInfo_FDS ={
	NULL,
	_T("Famicom Disk System"),
	COMPAT_FULL,
	NULL,
	FDS::reset,
	NULL,
	NULL,
	NULL,
	FDS::saveLoad,
	FDS::mapperSnd,
	NULL
};