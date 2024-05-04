#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define prgAND    (reg &0x04? 0x0F: 0x1F)
#define chrAND    (reg &0x02? 0x7F: 0xFF)
#define prgOR     (reg <<4 &0x10 | reg <<2 &0x20 | reg <<1 &0xC0)
#define chrOR     (reg <<7 &0x80 | reg <<4 &0x700)
#define tlsrom  !!(reg &0x80)

uint8_t		reg;

void	sync (void) {
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	if (tlsrom)
		MMC3::syncMirrorA17();
	else
		MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =755;
} // namespace

MapperInfo MapperInfo_755 = {
	&mapperNum,
	_T("ET-149"),
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