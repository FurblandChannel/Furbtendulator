#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t PRG, Mirroring;

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM8(0x8, PRG);
	EMU->SetPRG_ROM8(0xA, 0xFD);
	EMU->SetPRG_ROM8(0xC, 0xFE);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	EMU->SetCHR_RAM8(0, 0);
	if (Mirroring)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	PRG =Val;
	Sync();
}

void	MAPINT	WriteMirroring (int Bank, int Addr, int Val) {
	Mirroring =Val &0x8;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	FDSsound::ResetBootleg(ResetType);
	EMU->SetCPUWriteHandler(0x8, WritePRG);
	EMU->SetCPUWriteHandler(0xF, WriteMirroring);

	if (ResetType ==RESET_HARD) PRG =Mirroring =0;
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	offset = FDSsound::SaveLoad(mode, offset, data);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =309;
} // namespace

MapperInfo MapperInfo_309 ={
	&MapperNum,
	_T("Whirlwind Manu LH51"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	FDSsound::GetBootleg,
	NULL,
};