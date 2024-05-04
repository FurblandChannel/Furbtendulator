#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	if (Reg &0x0C)
		MMC3::syncPRG(0x0F, (Reg &0xC0) >>2);
	else
		EMU->SetPRG_ROM32(0x8, ((Reg &0xC0) >>4) | (Reg &0x03));
	MMC3::syncCHR_RAM(0xFF, 0x00);
	if (Reg &0x20) {
		if (Reg &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
		MMC3::syncMirror();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	Reg =Val;
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
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =345;
} // namespace

MapperInfo MapperInfo_345 = {
	&MapperNum,
	_T("L6IN1"),
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