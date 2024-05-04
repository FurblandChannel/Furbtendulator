 #include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_N118.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	Sync() {
	EMU->SetPRG_RAM4(0x6, 0);
	EMU->SetPRG_ROM4(0x7, 15);
	EMU->SetPRG_ROM8(0x8, N118::reg[6]);
	EMU->SetPRG_ROM4(0xA, 28);
	EMU->SetPRG_RAM4(0xB, 1);
	EMU->SetPRG_ROM8(0xC, N118::reg[7]);
	EMU->SetPRG_ROM8(0xE, 15);
	EMU->SetCHR_RAM8(0, 0);
	EMU->Mirror_Custom(N118::reg[2] &1, N118::reg[4] &1, N118::reg[3] &1, N118::reg[5] &1);
}

BOOL	MAPINT	Load (void) {
	N118::load(Sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	N118::reset(ResetType);
	FDSsound::ResetBootleg(ResetType);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset =N118::saveLoad(mode, offset, data);
	offset =FDSsound::SaveLoad(mode, offset, data);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =307;
} // namespace

MapperInfo MapperInfo_307 = {
	&MapperNum,
	_T("Kaiser KS-7037"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	FDSsound::GetBootleg,
	NULL
};