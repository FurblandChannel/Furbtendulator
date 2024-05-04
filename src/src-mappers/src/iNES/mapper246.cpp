#include	"..\DLL\d_iNES.h"

namespace {
FCPURead _Read6, _ReadF;
FCPUWrite _Write6;
uint8_t PRG[4], CHR[4], *PRGROM;

void	Sync (void) {
	for (int i =0; i <4; i++) {
		EMU->SetPRG_ROM8(0x8 +i*2, PRG[i]);
		EMU->SetCHR_ROM2(i*2, CHR[i]);
	}
	EMU->SetPRG_RAM4(0x6, 0);
}

int	MAPINT	Read6 (int Bank, int Addr) {
	if (Addr &0x800)
		return _Read6(Bank, Addr & 0x7FF);
	else	
		return -1;
}

int	MAPINT	ReadF (int Bank, int Addr) {
	// PRG ROM access when A15..A5 AND A2 are all HIGH should instead occur from a bank where PRG A17 is high, i.e. ( [$6003] | $10 )
	// FEDC BA98 7654 3210
	// 1111 1111 1110 0100
	if ((Addr &0xFE4) ==0xFE4)
		return PRGROM[( ((PRG[3] |0x10) <<13) | 0x1000 | Addr) &0x7FFFF];
	else
		return _ReadF(Bank, Addr);
}

void	MAPINT	Write6 (int Bank, int Addr, int Val) {
	if (Addr &0x800)
		_Write6(Bank, Addr & 0x7FF, Val);
	else if (Addr &4)
		CHR[Addr &3] =Val;
	else
		PRG[Addr &3] =Val;
	Sync();
}

BOOL	MAPINT	Load (void) {
	if (CART_BATTERY) EMU->Set_SRAMSize(2048);
	EMU->SetPRG_ROM8(0x8, 0);
	PRGROM =EMU->GetPRG_Ptr4(0x8);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();

	_Read6 = EMU->GetCPUReadHandler(0x6);
	_ReadF = EMU->GetCPUReadHandler(0xF);
	_Write6 = EMU->GetCPUWriteHandler(0x6);
	EMU->SetCPUReadHandler(0x6, Read6);
	EMU->SetCPUReadHandlerDebug(0x6, Read6);
	EMU->SetCPUWriteHandler(0x6, Write6);
	EMU->SetCPUReadHandler(0xF, ReadF);
	EMU->SetCPUReadHandlerDebug(0xF, ReadF);

	if (ResetType == RESET_HARD) {
		PRG[0] = 0x00;
		PRG[1] = 0x01;
		PRG[2] = 0xFE;
		PRG[3] = 0xFF;
		for (int i =0; i <4; i++) CHR[i] =0;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =246;
} // namespace

MapperInfo MapperInfo_246 ={
	&MapperNum,
	_T("C&E Fengshengban"),
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