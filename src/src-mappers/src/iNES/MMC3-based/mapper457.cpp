#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

/* Mapper 2786 in Nestopia Plus! */
namespace {
uint8_t		reg;
#define	prgOR     (reg <<4)
#define	chrOR     (reg <<7)
#define	prgAND    (reg &0x08? 0x1F: 0x0F)
#define	chrAND    (reg &0x08? 0xFF: 0x7F)

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
	MMC3::wramWrite(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x00;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =457;
} // namespace

MapperInfo MapperInfo_457 ={
	&mapperNum,
	_T("810431C"),
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