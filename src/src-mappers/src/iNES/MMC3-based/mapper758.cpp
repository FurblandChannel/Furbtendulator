#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
FCPURead	readCart;

int	MAPINT	readPad (int bank, int addr) {
	return readCart(bank, addr &~0x0F | ROM->dipValue &0x0F);
}

void	sync (void) {
	if (reg &0x02)
		MMC3::syncPRG_GNROM_66(reg &0x01? 2: 0, 0x0F, reg <<1 &0x70);
	else
		MMC3::syncPRG(reg &0x20? 0x1F: 0x0F, reg <<1 &0x70);
	
	MMC3::syncCHR(0xFF, reg <<4 &0x380);
	MMC3::syncMirror();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x40? readPad: readCart);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readCart =EMU->GetCPUReadHandler(0x8);
	reg =0;
	MMC3::reset(resetType);	
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =758;
} // namespace

MapperInfo MapperInfo_758 ={
	&mapperNum,
	_T("T-227"), /* 1993 Super HiK 60000-in-1 */
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