#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t	Reg, Mirroring, IRQEnabled, IRQActive;
uint16_t IRQCount;
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Reg);
	EMU->SetPRG_ROM16(0xC, 0x02);
	EMU->SetPRG_RAM8(6, 0);
	EMU->SetCHR_RAM8(0, 0);
	if (Mirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_BYTE(mode, offset, data, IRQActive);
	SAVELOAD_WORD(mode, offset, data, IRQCount);
	offset = FDSsound::SaveLoad(mode, offset, data);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

int	MAPINT	Read (int Bank, int Addr) {
	if (Addr ==0x30) {
		EMU->SetIRQ(1);
		int result =IRQActive;
		IRQActive =0;
		return result;
	} else if (Addr < 0x20)
		return _Read4(Bank, Addr);
	else
		return FDSsound::Read((Bank <<12) |Addr);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Bank ==4 && Addr <0x20)
		_Write4 (Bank, Addr, Val);
	else {
		if (Bank ==4 && (Addr &0xF00) ==0xA00) Reg = ((Addr >>2) &3) | ((Addr >>4) &4); else
		if (Bank ==5 && (Addr &0xF00) ==0x100) Sync(); else
		if (Bank ==4 && Addr ==0x20) { EMU->SetIRQ(1); IRQCount = IRQCount &0xFF00 |  Val     ;                } else
		if (Bank ==4 && Addr ==0x21) { EMU->SetIRQ(1); IRQCount = IRQCount &0x00FF | (Val <<8); IRQEnabled =1; } else
		if (Bank ==4 && Addr ==0x25) { Mirroring = Val &8; } else
		FDSsound::Write((Bank <<12) |Addr, Val);
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	FDSsound::Reset(ResetType);
	if (ResetType == RESET_HARD) Reg =IRQEnabled =IRQActive =IRQCount =0;

	_Read4 =EMU->GetCPUReadHandler(4);
	_Write4 =EMU->GetCPUWriteHandler(4);
	EMU->SetCPUReadHandler(4, Read);
	EMU->SetCPUReadHandler(5, Read);
	EMU->SetCPUWriteHandler(4, Write);
	EMU->SetCPUWriteHandler(5, Write);
	Sync();
}

void	MAPINT	CPUCycle (void) {
	if (IRQEnabled && !--IRQCount) {
		IRQEnabled =0;
		IRQActive =1;
		EMU->SetIRQ(0);
	}
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Unload (void) {
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =303;
} // namespace

MapperInfo MapperInfo_303 = {
	&MapperNum,
	_T("Kaiser KS-7017"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	CPUCycle,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL
};