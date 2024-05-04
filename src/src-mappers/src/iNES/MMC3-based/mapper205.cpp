#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t OuterBank;

void	Sync (void) {
	if (OuterBank &2) {
		MMC3::syncPRG(0x0F, OuterBank <<4);
		MMC3::syncCHR_ROM(0x7F, OuterBank <<7);
	} else {
		MMC3::syncPRG(0x1F, OuterBank <<4);
		MMC3::syncCHR_ROM(0xFF, OuterBank <<7);
	}
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	MMC3::wramWrite(Bank, Addr, Val);
	OuterBank =Val &3;
	if (OuterBank &1 && ROM->dipValue) OuterBank |=2;
	Sync();
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	OuterBank =0;
	MMC3::reset(ResetType);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUWriteHandler(i, WriteReg);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =205;
} // namespace

MapperInfo MapperInfo_205 = {
	&MapperNum,
	_T("JC-016-2"),
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