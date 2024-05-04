#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[4];

void	sync (void) {
	int prgAND =reg[0] &0x20? (0xFF >> (7 -(reg[0] &7))): 0x00;
	int prgOR  =reg[0] &0x20? (reg[1] | reg[2] <<8): 0x3F;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	if (~reg[0] &0x80)
		EMU->SetCHR_ROM8(0x0, reg[3]);
	else
		MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
	MMC3::syncWRAM();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(0x8, reg[0] &0x40? EMU->WritePRG: MMC3::write);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr &0x800) {
		reg[addr &3] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =473;
} // namespace

MapperInfo MapperInfo_473 ={
	&mapperNum,
	_T("KJ01A-18"),
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
