#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
void	sync (void) {	
	int  prgOR =reg <<4 &0x70;
	int  chrOR =ROM->INES2_SubMapper ==14? (reg <<3 &0x080 | reg <<7 &0x300): (reg <<3 &0x180 | reg <<7 &0x200);
	int  prgAND =reg &0x08? 0x0F: 0x1F;
	int  chrAND =reg &0x40? 0x7F: 0xFF;
	bool chrram =ROM->INES2_SubMapper ==13 && (reg &0x3) ==0x3 ||
	             ROM->INES2_SubMapper ==14 && !!(reg &0x20);

	MMC3::syncWRAM();
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	if (chrram)
		EMU->SetCHR_RAM8(0x0, 0);
	else
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg &0x80) {
		reg =val;
		sync();
	} else
		MMC3::wramWrite(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::NEC);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x00;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =52;
} // namespace

MapperInfo MapperInfo_052 ={
	&mapperNum,
	_T("Realtec 8213"),
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