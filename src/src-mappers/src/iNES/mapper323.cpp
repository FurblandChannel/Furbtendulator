#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC1.h"

namespace {
uint8_t	Reg;
void	Sync (void) {
	uint8_t OuterBank =(Reg &0xF0) >>4;
	MMC1::syncMirror();
	MMC1::syncPRG(0x07, OuterBank <<3);
	MMC1::syncCHR_ROM(0x1F, OuterBank <<5);
}

void	MAPINT	Write67 (int Bank, int Addr, int Val) {
	if (~MMC1::reg[3] &0x10 && ~Reg &8) {
		Reg =Val;
		Sync();
	}
};

BOOL	MAPINT	Load (void) {
	MMC1::load(Sync, MMC1Type::MMC1B);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) Reg =0;
	MMC1::reset(ResetType);
	EMU->SetCPUWriteHandler(6, Write67);
	EMU->SetCPUWriteHandler(7, Write67);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	offset = MMC1::saveLoad(mode, offset, data);
	if (mode == STATE_LOAD) Sync();
	return offset;
}


uint16_t MapperNum =323;
} // namespace

MapperInfo MapperInfo_323 = {
	&MapperNum,
	_T("FARID SLROM"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
