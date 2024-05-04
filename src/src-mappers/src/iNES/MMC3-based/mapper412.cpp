#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define locked  (reg[1] &0x01)
#define	prgAND  (0x0F | (reg[1] &0x02? 0x00: 0x10) | (reg[1] &0x10? 0x00: 0x20))
#define	chrAND  (reg[1] &0x20? 0x7F: 0xFF)
#define prgOR   ((reg[1] &0x40? 0x10: 0x00) | (reg[1] &0x04? 0x20: 0x00))
#define chrOR   ((reg[1] &0x80? 0x80: 0x00) | (reg[1] &0x08? 0x100: 0x000))
#define nrom      !!(reg[2] &0x02)
#define nrom256   !!(reg[2] &0x04)
#define prg         (reg[2] >>3)
#define chr         (reg[0] >>2)

namespace {
uint8_t		reg[4];

void	sync (void) {
	MMC3::wram =0x80;
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, prg &~(1*nrom256));
		EMU->SetPRG_ROM16(0xC, prg |  1*nrom256 );
		EMU->SetCHR_ROM8(0x0, chr);
	} else {
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	}
		
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if (!locked) {
		reg[addr &3] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(RESET_HARD);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =412;
} // namespace

MapperInfo MapperInfo_412 = {
	&mapperNum,
	_T("恒格 FK-206 JG"),
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
