#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t	Reg[4], *PRG;
FCPURead _Read4;
FCPUWrite _Write4;

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

int	MAPINT	Read67 (int Bank, int Addr) {
	int index= ((Bank -6) <<1) | ((Addr &0x800)? 1: 0);
	return PRG[(Reg[index] <<11) | (Addr &0x7FF)];
	
}

int	MAPINT	Read8F (int Bank, int Addr) {
	int index= ((Bank -8) <<1) | ((Addr &0x800)? 1: 0);
	return PRG[((15 -index) <<11) | (Addr &0x7FF)];
	
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	int index = ((Bank &1)? 2: 0) | ((Addr &0x800)? 1: 0);
	Reg[index] =Val &0x3F;
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->SetPRG_ROM32(8, 0);
	PRG =EMU->GetPRG_Ptr4(8);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();

	if (ResetType == RESET_HARD) for (int i =0; i <4; i++) Reg[i] =0;

	for (int i =0x6; i<=0x7; i++) EMU->SetCPUReadHandler(i, Read67);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUReadHandler(i, Read8F);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUReadHandlerDebug(i, Read67);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUReadHandlerDebug(i, Read8F);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);

	FDSsound::Reset(ResetType);
	_Read4 =EMU->GetCPUReadHandler(0x4);
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUWriteHandler(0x4, Write4);
}

void	MAPINT	Unload (void) {
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, Reg[i]);
	offset = FDSsound::SaveLoad(mode, offset, data);
	return offset;
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =305;
} // namespace

MapperInfo MapperInfo_305 = {
	&MapperNum,
	_T("Kaiser KS-7031"),
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