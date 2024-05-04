#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t OuterBank;

void	Sync (void) {
	MMC3::syncMirror();
	MMC3::syncPRG(0x0F, OuterBank &~0x0F);
	MMC3::syncCHR_ROM(0x7F, (OuterBank &~0x0F) <<3);
	MMC3::syncWRAM();
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}

void	MAPINT	WriteOuterBank (int Bank, int Addr, int Val) {
	MMC3::wramWrite(Bank, Addr, Val);
	if (~OuterBank &0x80) OuterBank =Addr &0xFF;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {	
	OuterBank =0;
	MMC3::reset(ResetType);
	EMU->SetCPUWriteHandler(0x6, WriteOuterBank);
	EMU->SetCPUWriteHandler(0x7, WriteOuterBank);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	offset =MMC3::saveLoad(mode, offset, data);
	if (mode ==STATE_LOAD)	Sync();
	return offset;
}

uint16_t MapperNum =366;
} // namespace

MapperInfo MapperInfo_366 ={
	&MapperNum,
	_T("GN-45"),
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