#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
FCPURead _Read4;
FCPUWrite _Write4;
uint8_t PRG;

void	Sync (void) {
	EMU->SetPRG_ROM8(0x6, PRG);
	EMU->SetPRG_ROM8(0x8, 0xFC);
	EMU->SetPRG_ROM8(0xA, 0xFD);
	EMU->SetPRG_RAM8(0xC, 0);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	EMU->SetCHR_RAM8(0, 0);
}

int	MAPINT	Read4 (int Bank, int Addr) {
	if (Addr <0x20)
		return _Read4(Bank, Addr);
	else
		return FDSsound::Read((Bank <<12) |Addr);
}

void	MAPINT	Write4 (int Bank, int Addr, int Val) {
	_Write4(Bank, Addr, Val);
	if (Addr !=0x23) // Don't relay Master IO Disable
		FDSsound::Write((Bank <<12) |Addr, Val);
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	PRG =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	FDSsound::Reset(ResetType);
	_Read4 =EMU->GetCPUReadHandler(0x4);
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUWriteHandler(0x4, Write4);
	EMU->SetCPUWriteHandler(0x6, WritePRG);

	if (ResetType ==RESET_HARD) PRG =0;
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Unload (void) {
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	offset = FDSsound::SaveLoad(mode, offset, data);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =125;
} // namespace

MapperInfo MapperInfo_125 ={
	&MapperNum,
	_T("Whirlwind Manu LH32"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL,
};