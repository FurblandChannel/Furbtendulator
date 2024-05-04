#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	uint8_t Mode =Reg &((ROM->dipValue && ROM->INES_PRGSize <=512/16)? 0xC: 0x8);
	uint8_t InnerBank =Reg >>4;
	uint8_t OuterBank =Reg &7;
	
	if (Mode)
		EMU->SetPRG_ROM32(0x8, InnerBank | (OuterBank <<2));
	else
		MMC3::syncPRG(0x0F, OuterBank <<4);
	MMC3::syncCHR_ROM(0x7F, OuterBank <<7);
	MMC3::syncMirror();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	Reg =Addr &0xFF;
	Sync();
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType == RESET_HARD) Reg =0;
	MMC3::reset(ResetType);
	MMC3::setWRAMCallback(NULL, WriteReg);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =287;
} // namespace

MapperInfo MapperInfo_287 = {
	&MapperNum,
	_T("811120-C/810849-C"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	SaveLoad,
	NULL,
	NULL
};