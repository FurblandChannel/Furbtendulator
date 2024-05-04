#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t 	outerBank;

void	sync (void) {
	int bank =((outerBank &0x20) >>2) | (outerBank &0x06);
	
	MMC3::syncMirror();
	MMC3::syncPRG(0x1F, bank <<4);
	MMC3::syncCHR_ROM(0x7F, bank <<6);
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (~outerBank &0x80) {
		outerBank =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	outerBank =0;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =267;
} // namespace

MapperInfo MapperInfo_267 ={
	&mapperNum,
	_T("晶太 EL861121C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};