#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
void	sync (void) {	
	int  prgOR =(reg &0x08? 0x020: 0x000) | (reg &0x40? 0x010: 0x000);
	int  chrOR =(reg &0x04? 0x100: 0x000) | (reg &0x02? 0x080: 0x000);
	int  prgAND =reg &0x20? 0x0F: 0x1F;
	int  chrAND =reg &0x01? 0xFF: 0x7F;

	MMC3::syncWRAM();
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg &0x80) {
		reg =val |0x80;
		sync();
	}
	MMC3::wramWrite(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
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

uint16_t mapperNum =750;
} // namespace

MapperInfo MapperInfo_750 ={
	&mapperNum,
	_T("FC-53A"), /* (F-651) 1994 New Super 3-in-1 */
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