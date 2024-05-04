#include	"..\..\DLL\d_iNES.h"

namespace {
FCPUWrite WriteAPU;
uint8_t	BankSMB2J, BankUNROM, BankSwap, IRQEnabled;
uint16_t IRQCounter;

static const uint8_t BankTranslate[2][8] = {
	{ 4, 3, 5, 3, 6, 3, 7, 3 },
	{ 1, 1, 5, 1, 4, 1, 5, 1 }
};

void	Sync (void) {
	if (ROM->dipValue ==0) {
		EMU->SetPRG_ROM8(0x6, BankSwap? 0: 2);
		EMU->SetPRG_ROM8(0x8, BankSwap? 0: 1);
		EMU->SetPRG_ROM8(0xA, 0);
		EMU->SetPRG_ROM8(0xC, BankTranslate[BankSwap][BankSMB2J]);
		EMU->SetPRG_ROM8(0xE, BankSwap? 8: 10);		
	} else {
		EMU->SetPRG_ROM16(0x8, ROM->dipValue | BankUNROM);
		EMU->SetPRG_ROM16(0xC, ROM->dipValue | 7);
	}
	iNES_SetCHR_Auto8(0x0, 0);
	if (ROM->dipValue ==0x18)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Bank ==4) WriteAPU(Bank, Addr, Val); else
	if (Bank &8) {
		BankUNROM =Val &0x7;
		Sync();
	}

	Addr |=Bank <<12;
	if ((Addr &0x71FF) ==0x4022) { 
		BankSMB2J =Val &0x7; 
		Sync(); 
	}
	if ((Addr &0x71FF) ==0x4120) { 
		BankSwap =Val &1; 
		Sync(); 
	}
	if ((Addr &0xF1FF) ==0x4122) { 
		IRQEnabled =Val &1; 
		IRQCounter =0;
		EMU->SetIRQ(1); 
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) BankSMB2J =3;
	IRQEnabled =IRQCounter =0;
	Sync();

	WriteAPU =EMU->GetCPUWriteHandler(0x4);
	for (int i =0x4; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
}

void	MAPINT	CPUCycle (void) {
	if (IRQEnabled && !(++IRQCounter &4095)) EMU->SetIRQ(0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, BankSMB2J);
	SAVELOAD_BYTE(mode, offset, data, BankUNROM);
	SAVELOAD_BYTE(mode, offset, data, BankSwap);
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_WORD(mode, offset, data, IRQCounter);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =357;
} // namespace

MapperInfo MapperInfo_357 ={
	&MapperNum,
	_T("普澤 P3117"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
