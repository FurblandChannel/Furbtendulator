#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define nrom    !!(reg &0x04)
#define dip     !!(reg &0x08)
#define prgAND     0x0F
#define chrAND    (reg &0x02? 0x7F: 0xFF)
#define prgOR     (reg <<4)
#define chrOR     (reg <<7)

uint8_t		reg;
FCPURead	readCart;

void	sync (void) {
	MMC3::syncPRG_NROM(prgAND, prgOR &~prgAND, !nrom*3, nrom*3);
	MMC3::syncCHR(chrAND, chrOR);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

int	MAPINT	trapCartRead (int bank, int addr) {
	if (reg ==0x8)
		return ROM->dipValue;
	else
		return readCart(bank, addr);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	MMC3::setWRAMCallback(NULL, writeReg);

	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, trapCartRead);
		EMU->SetCPUReadHandlerDebug(bank, trapCartRead);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =344;
} // namespace

MapperInfo MapperInfo_344 = {
	&mapperNum,
	_T("GN-26"),
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