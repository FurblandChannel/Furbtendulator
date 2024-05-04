#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define dip       (ROM->INES2_SubMapper ==1? !!(reg &0x80): !!(reg &0x20))
#define nrom256   (ROM->INES2_SubMapper ==1? !!(reg &0x08): !!(reg &0x10))
#define prg       (reg &0x0F)

namespace {
uint8_t		reg;

FCPURead	readCart;
int	MAPINT	readDIP (int bank, int addr);

void	sync (void) {	
	EMU->SetPRG_ROM16(0x8, prg &~(nrom256));
	EMU->SetPRG_ROM16(0xC, prg |  nrom256 );
	MMC3::syncCHR_ROM(0x7F, prg <<4 &~0x7F);
	MMC3::syncMirror();
	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, dip? readDIP: readCart);
}

int	MAPINT	readDIP (int bank, int addr) {
	return readCart(bank, addr &~0x1F | ROM->dipValue);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
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
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =458;
} // namespace

MapperInfo MapperInfo_458 ={
	&mapperNum,
	_T("GN-23"), /* 93年世界玩具展優良卡匣 10000-in-1 */
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