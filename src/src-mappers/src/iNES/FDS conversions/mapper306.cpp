#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t PRG;
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync() {
	EMU->SetPRG_ROM8(0x6, (PRG >11)? 11: PRG);
	EMU->SetPRG_ROM32(0x8, 3);
	EMU->SetCHR_RAM8(0, 0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	offset = FDSsound::SaveLoad(mode, offset, data);
	return offset;
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

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if ((Addr &0x903) ==0x903) {
		if (Addr &0x40)			
			PRG = (Addr >>2) &0x0F;
		else
			PRG =((Addr >>2) &0x03) |0x08;
		Sync();
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	if (ResetType ==RESET_HARD) PRG =0;

	EMU->SetCPUWriteHandler(0xD, Write);
	EMU->SetCPUWriteHandler(0xF, Write);

	FDSsound::Reset(ResetType);
	_Read4 =EMU->GetCPUReadHandler(0x4);
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUWriteHandler(0x4, Write4);
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Unload (void) {
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =306;
} // namespace

MapperInfo MapperInfo_306 = {
	&MapperNum,
	_T("Kaiser KS-7016"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL
};