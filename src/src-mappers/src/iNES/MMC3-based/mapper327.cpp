#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	int OuterBank =Reg &7;
	if (Reg &0x08)
		MMC3::syncPRG(0x1F, OuterBank <<4);
	else
		MMC3::syncPRG(0x0F, OuterBank <<4);
	
	if (Reg &0x10)
		MMC3::syncCHR_RAM(0xFF, 0x00);
	else if (Reg &0x20)
		MMC3::syncCHR_ROM(0xFF, OuterBank <<7);
	else
		MMC3::syncCHR_ROM(0x7F, OuterBank <<7);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	MMC3::wramWrite(Bank, Addr, Val);
	if ((Reg &7) ==0) {
		Reg =Addr;
		Sync();
	}
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

uint16_t MapperNum =327;
} // namespace

MapperInfo MapperInfo_327 = {
	&MapperNum,
	_T("10-24-C-A1"),
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