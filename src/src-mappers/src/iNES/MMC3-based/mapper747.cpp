#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define prgAND    (reg &0x10? 0x1F: 0x0F)
#define chrAND    (reg &0x10? 0xFF: 0x7F)
#define prgOR     (reg <<1 &0x3E)
#define chrOR     (reg <<4 &0x180)
#define prg       (reg &0x1F)
#define nrom     !(reg &0x20)
#define nrom256 !!(reg &0x06)
#define lock    !!(reg &0x20)

uint8_t		reg;

void	sync (void) {
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, prg &~nrom256);
		EMU->SetPRG_ROM16(0xC, prg | nrom256);
	} else
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if (!lock) {
		reg =addr &0xFF;
		sync();
	}
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

uint16_t mapperNum =747;
} // namespace

MapperInfo MapperInfo_747 = {
	&mapperNum,
	_T("K-3022"), /* 32-in-1 高K 瑪琍兄弟大全 */
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