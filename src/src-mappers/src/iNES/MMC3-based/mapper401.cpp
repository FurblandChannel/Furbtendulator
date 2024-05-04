#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prgAND ~reg[3] &0x1F
#define chrAND  0xFF >>(~reg[2] &0xF)
#define prgOR   reg[1] &0x1F | reg[2] &0x80 | \
	       (ROM->dipValue &2? reg[2] &0x20: reg[1] >>1 &0x20) |\
	       (ROM->dipValue &4? reg[2] &0x40: reg[1] <<1 &0x40)
#define prgCE   ROM->dipValue &1 && reg[1] &0x80
#define chrOR   reg[0]       | reg[2] <<4 &0xF00
#define locked (reg[3] &0x40)

namespace {
uint8_t		index;
uint8_t		reg[4];

void	sync (void) {
	MMC3::syncWRAM();
	if (prgCE)
		for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
	else
		MMC3::syncPRG(prgAND, prgOR);

	MMC3::syncCHR_ROM(chrAND, chrOR);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if (!locked) {
		reg[index++ &3] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	index =0;
	reg[0] =0x00;
	reg[1] =0x00;
	reg[2] =0x0F;
	reg[3] =0x00;
	MMC3::reset(resetType);

	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =401;
} // namespace

MapperInfo MapperInfo_401 ={
	&mapperNum,
	_T("KC885"),
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
