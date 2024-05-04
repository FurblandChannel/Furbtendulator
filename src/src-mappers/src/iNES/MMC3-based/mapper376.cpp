#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	reg[2];

void	sync (void) {
	int outerPRG =(reg[0] &0x07) | (reg[0] &0x40? 0x08: 0x00) | (reg[1] &0x01? 0x10: 0x00);
	int outerCHR =(reg[0] &0x40? 0x080: 0x000) | (reg[1] &0x01? 0x100: 0x000);
	
	if (reg[0] &0x80) {
		if (reg[0] &0x20)
			EMU->SetPRG_ROM32(0x8, outerPRG >>1);
		else {
			EMU->SetPRG_ROM16(0x8, outerPRG);
			EMU->SetPRG_ROM16(0xC, outerPRG);
		}
	} else
		MMC3::syncPRG(0x0F, (outerPRG <<1) &~0x0F);
	MMC3::syncCHR(0x7F, outerCHR);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	if (resetType ==RESET_HARD) {
		reg[0] =0;
		reg[1] =0;
	}
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x7, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg[0]);
	SAVELOAD_BYTE(stateMode, offset, data, reg[1]);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =376;
} // namespace

MapperInfo MapperInfo_376 ={
	&mapperNum,
	_T("晶太 YY841155C"),
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