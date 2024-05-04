#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	if (reg &0x80)
		MMC3::syncPRG(0x1F, reg >>1 &0x20);
	else
	if (reg &0x20)
		EMU->SetPRG_ROM32(0x8, (reg &0x0F | reg >>2 &0x10) >>1);
	else {
		EMU->SetPRG_ROM16(0x8,  reg &0x0F | reg >>2 &0x10);
		EMU->SetPRG_ROM16(0xC,  reg &0x0F | reg >>2 &0x10);
	}
	EMU->SetCHR_RAM8(0, 0);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x0F;
	MMC3::reset(resetType);
	MMC3::wram |=0x80;
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =745;
} // namespace

MapperInfo MapperInfo_745 ={
	&mapperNum,
	_T("JY-014"), /*(CG-008) 190-in-1 1994 年冠軍精選超值享受 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};