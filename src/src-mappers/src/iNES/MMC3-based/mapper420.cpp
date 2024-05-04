#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[4];

#define nrom    !!(reg[0] &0x80)
#define prg256k !!(reg[3] &0x20)
#define prg128k !!(reg[0] &0x20)
#define prgA18  !!(reg[3] &0x04)
#define prgNROM   (reg[0] >>1 &7)
#define chr128k !!(reg[1] &0x80)
#define chrA18  !!(reg[1] &0x80)
#define chrA17  !!(reg[1] &0x04)
// Unknown bits: prgA17 in MMC3 mode, NROM-128?

void	sync (void) {
	if (nrom)
		EMU->SetPRG_ROM32(0x8, prgNROM | prg256k *8);
	else
		MMC3::syncPRG(prg128k? 0x0F: prg256k? 0x1F: 0x3F, prgA18 *0x20);
	MMC3::syncCHR(chr128k? 0x7F: 0xFF, chrA18 *0x100 | chrA17 *0x80);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	reg[addr &3] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(resetType);
	for (int bank =0x6; bank <0x8; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =420;
} // namespace

MapperInfo MapperInfo_420 ={
	&mapperNum,
	_T("卡聖 A971210"),
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