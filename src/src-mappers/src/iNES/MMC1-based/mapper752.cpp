#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		reg;
FCPUWrite	writeWRAM;
	
void	sync (void) {
	if (reg &4) {
		MMC1::syncPRG(reg &0x08? 0x07: 0x03, reg &0x0C);
		MMC1::syncCHR(0x1F, reg <<2 &0x20);
	} else {
		EMU->SetPRG_ROM16(0x8, reg &0x03);
		EMU->SetPRG_ROM16(0xC, reg &0x03);
		MMC1::syncCHR(0x07, 0x40);
	}
	MMC1::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC1::load(sync, MMC1Type::MMC1A);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC1::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =752;
} // namespace

MapperInfo MapperInfo_752 ={
	&mapperNum,
	_T("CM-9309"), /* (GameStar) Wuzi Gun 6-in-1 */
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
