#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	PRG;

void	Sync (void) {
	EMU->SetPRG_ROM32(0x8, PRG >>1);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

int	MAPINT	ReadDIP (int Bank, int Addr) {
	if (Addr &2)
		return (*EMU->OpenBus &0xFE) | (ROM->dipValue &0x01);
	else
		return *EMU->OpenBus;
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	if (~Addr &1) {
		PRG =Val;
		Sync();
	}
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	MMC3::reset(ResetType);
	MMC3::setWRAMCallback(ReadDIP, WritePRG);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, PRG);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =334;
} // namespace

MapperInfo MapperInfo_334 ={
	&MapperNum,
	_T("821202C"),
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