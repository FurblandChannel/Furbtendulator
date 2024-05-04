#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	MMC3::pointer &=0x3F;
	if (reg &0x20) {
		int prgAND =reg &0x40? 0x0F: 0x03;
		int prgOR  =reg <<1;
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	} else {
		EMU->SetPRG_ROM16(0x8, reg &0x1F);
		EMU->SetPRG_ROM16(0xC, reg &0x1F);
	}
	if (reg &0x40) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(1) | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(5) | reg <<2 &0x100);
	} else {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) &~3 | 0 | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(1) &~3 | 1 | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) &~3 | 2 | reg <<2 &0x100);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(5) &~3 | 3 | reg <<2 &0x100);
	}
	if (reg &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);	
	for (int bank =0x9; bank <=0xF; bank +=2) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =467;
} // namespace

MapperInfo MapperInfo_467 ={
	&mapperNum,
	_T("47-2"),
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