#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prgOR     reg[0]
#define chrOR    (reg[1] <<3)
#define prgAND   (reg[2] &0x01? 0x0F: 0x1F)
#define chrAND   (reg[2] &0x08? 0x7F: 0xFF)
#define locked !!(reg[3] &0x20)

namespace {
uint8_t		reg[4];

void	sync (void) {
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
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
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);	
	EMU->SetCPUWriteHandler(0x5, writeReg);	
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =445;
} // namespace

MapperInfo MapperInfo_445 ={
	&mapperNum,
	_T("DG574B"),
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