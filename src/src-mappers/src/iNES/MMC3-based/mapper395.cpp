#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[2];
#define	locked (reg[1] &0x80)
#define	prgAND (reg[1] &0x08? 0x0F: 0x1F)
#define	chrAND (reg[1] &0x40? 0x7F: 0xFF)
#define	prgOR  (reg[0] <<1 &0x60 | reg[0] <<4 &0x80 | reg[1] <<4 &0x10)
#define	chrOR  (reg[0] <<4 &0x300| reg[1] <<5 &0x400| reg[1] <<3 &0x80)

void	sync (void) {
	MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR(chrAND, chrOR);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if (!locked) {
		reg[addr >>4 &1] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =395;
} // namespace

MapperInfo MapperInfo_395 ={
	&mapperNum,
	_T("Realtec 8210"),
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