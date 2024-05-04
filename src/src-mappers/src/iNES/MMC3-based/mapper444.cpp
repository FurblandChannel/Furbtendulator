#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define chrA17_MMC3 !!(ROM->INES2_SubMapper &1)
#define chrA18_D4   !!(ROM->INES2_SubMapper &2)
#define prgAND        (ROM->INES2_SubMapper &4 && reg &0x02? 0x1F: 0x0F)
#define chrAND        (chrA17_MMC3? 0xFF: 0x7F)
#define prgOR         (reg <<4)
#define chrOR         (reg <<7 &(chrA17_MMC3? 0x000: 0x080) | reg <<(chrA18_D4? 4: 7) &0x100)
#define nrom        !!(reg &0x04)
#define nrom128     !!(reg &0x08)

uint8_t		reg;
FCPURead	readCart;

void	sync (void) {
	MMC3::syncPRG_NROM(prgAND, prgOR &~prgAND, !nrom*3, nrom*1 | nrom*!nrom128*2);
	MMC3::syncCHR(chrAND, chrOR);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

int	MAPINT	trapCartRead (int bank, int addr) {
	if (!nrom && nrom128)
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

uint16_t mapperNum =444;
} // namespace

MapperInfo MapperInfo_444 = {
	&mapperNum,
	_T("NC7000M"),
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