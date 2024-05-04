#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	OuterBank;

void	Sync (void) {
	MMC3::syncPRG(0x0F, (OuterBank &0x3) <<4);
	MMC3::syncCHR_ROM(0x7F, (OuterBank &0xC) <<5);
	MMC3::syncMirror();
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Addr &0x10)	OuterBank =Addr &0xF;
	MMC3::write(Bank, Addr, Val);
}

void	MAPINT	Reset (RESET_TYPE ResetType) {	
	if (ResetType ==RESET_HARD) OuterBank =0;
	MMC3::reset(ResetType);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	offset = MMC3::saveLoad(mode, offset, data);
	if (mode == STATE_LOAD)	Sync();
	return offset;
}

uint16_t MapperNum =516;
} // namespace

MapperInfo MapperInfo_516 ={
	&MapperNum,
	_T("COCOMA"),
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