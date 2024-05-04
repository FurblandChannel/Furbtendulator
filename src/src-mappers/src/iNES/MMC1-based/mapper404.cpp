#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

#define prgAND   (reg &0x40? 0x07: 0x0F)
#define prgOR    (reg <<3)
#define chrOR    (reg <<5)
#define locked !!(reg &0x80)

namespace {
uint8_t		reg;

void	sync (void) {
	MMC1::syncPRG(prgAND, prgOR &~prgAND);
	MMC1::syncCHR_ROM(0x1F, chrOR);
	MMC1::syncMirror();
	MMC1::syncWRAM(0);
}

void	MAPINT	writereg (int bank, int addr, int val) {
	if (!locked) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1A);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x00;
	MMC1::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writereg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =404;
} // namespace

MapperInfo MapperInfo_404 = {
	&mapperNum,
	_T("JY012005"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};

