#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	if (Reg &0x20)
		EMU->SetPRG_ROM32(8, ((Reg &0x1F) >>1) | ((Reg &0x40)? 0x04: 0x00));
	else
		MMC3::syncPRG(0x0F, (Reg &0x40)? 0x10: 0x00);
	MMC3::syncCHR_ROM(0xFF, (Reg &0x40)? 0x100: 0x000);
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
	EMU->SetCPUWriteHandler(0x6, WriteReg);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum = 291;
} // namespace

MapperInfo MapperInfo_291 = {
	&MapperNum,
	_T("Super 2-in-1"),
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