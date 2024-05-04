#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead _Read;
uint8_t	Lock, CHR, Scratch[4];

void	Sync (void) {
	if (!Lock) {
		Lock =(Latch::addr &0x100)? 1: 0;
		
		if (Latch::addr &0x80) {
			EMU->SetPRG_ROM16(0x8, Latch::addr);
			EMU->SetPRG_ROM16(0xC, Latch::addr);
		}
		else
			EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
		
		if (Latch::data &0x80)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
		
		CHR = Latch::data &0x7C;
	}
	iNES_SetCHR_Auto8(0, CHR | (Latch::data &0x03));
}

int	MAPINT	ReadDIP (int Bank, int Addr) {
	if (Latch::addr &0x40) Addr =(Addr &0xFF0) | ROM->dipValue;
	return _Read(Bank, Addr);
}

int	MAPINT	Read5 (int Bank, int Addr) {
	if (Addr &0x800)
		return Scratch[Addr &3];
	else
		return *EMU->OpenBus;
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	if (Addr &0x800)
		Scratch[Addr &3] =Val &0xF;
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) for (int i =0; i <4; i++) Scratch[i] =0;
	Lock =0;
	iNES_SetMirroring();
	Latch::reset(RESET_HARD);
	EMU->SetCPUReadHandler(5, Read5);
	EMU->SetCPUWriteHandler(5, Write5);
	_Read =EMU->GetCPUReadHandler(8);
	for (int i =0x8; i <=0xF; i++) EMU->SetCPUReadHandler(i, ReadDIP);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_AD(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, Lock);
	SAVELOAD_BYTE(mode, offset, data, CHR);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum = 519;
} // namespace

MapperInfo MapperInfo_519 =
{
	&MapperNum,
	_T("EH8813A"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
