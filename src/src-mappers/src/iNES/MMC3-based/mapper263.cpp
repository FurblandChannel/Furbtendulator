#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {	
void	Sync (void) {
	MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	MMC3::write(Bank &~1, Addr | (Bank &1), (Val &0xD8) | ((Val &0x20) >>4) | ((Val &4) <<3) | ((Val &2) >>1) | ((Val &1) <<2));
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	MMC3::reset(ResetType);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
}

uint16_t MapperNum =263;
} // namespace

MapperInfo MapperInfo_263 = {
	&MapperNum,
	_T("S.M.I. NSM-xxx"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};
