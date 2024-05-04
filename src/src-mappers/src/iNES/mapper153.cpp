#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_LZ93D50.h"

namespace {
uint8_t		OuterBank;

void	Sync (void) {
	LZ93D50::syncPRG(0x0F, OuterBank <<4);
	EMU->SetCHR_RAM8(0x00, 0);
	LZ93D50::syncMirror();
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	LZ93D50::load(Sync, NULL, false);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) OuterBank =0;
	LZ93D50::reset(ResetType);
}

void	MAPINT	PPUCycle (int Addr, int Scanline, int Cycle, int IsRendering) {
	uint8_t NewOuterBank =LZ93D50::chr[(Addr >>10) &3];
	if (NewOuterBank !=OuterBank) {
		OuterBank =NewOuterBank;
		Sync();
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset =LZ93D50::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	return offset;
}

uint16_t MapperNum =153;
} // namespace

MapperInfo MapperInfo_153 ={
	&MapperNum,
	_T("Bandai LZ93D50 with 8 KiB WRAM"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	LZ93D50::cpuCycle,
	PPUCycle,
	SaveLoad,
	NULL,
	NULL
};