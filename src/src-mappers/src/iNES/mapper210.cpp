#include	"..\DLL\d_iNES.h"

namespace {
uint8_t PRG[4], CHR[8], WRAMWriteEnable;
uint16_t WRAMAddressMask;
FCPURead _ReadWRAM[2];
FCPUWrite _WriteWRAM[2];

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0x6; i<=0x7; i++) EMU->SetPRG_Ptr4(i, EMU->GetPRG_Ptr4(i), WRAMWriteEnable? TRUE: FALSE);
	for (int i =0; i <4; i++) EMU->SetPRG_ROM8(8 +i*2, PRG[i] &0x3F);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, CHR[i]);
	if (ROM->INES2_SubMapper !=1) switch(PRG[0] >>6) {
		case 0:	EMU->Mirror_S0(); break;
		case 1:	EMU->Mirror_V(); break;
		case 2:	EMU->Mirror_H(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

// The mapper interface does not wrap WRAM sizes smaller than 2 KiB correctly, so restrict the address bits by hand.
int	MAPINT	ReadWRAM (int Bank, int Addr) { 
	return _ReadWRAM[Bank &1](Bank, Addr &WRAMAddressMask);
}

void	MAPINT	WriteWRAM (int Bank, int Addr, int Val) {
	_WriteWRAM[Bank &1](Bank, Addr &WRAMAddressMask, Val);
}
void	MAPINT	WriteWRAMEnable (int Bank, int Addr, int Val) {
	if (~Addr &0x800) WRAMWriteEnable =Val &1;
	Sync();
}

void	MAPINT	WriteCHR (int Bank, int Addr, int Val) {
	int Reg =((Bank &3) <<1) | ((Addr &0x800)? 1: 0);
	CHR[Reg] =Val;
	Sync();
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	int Reg =((Bank &1) <<1) | ((Addr &0x800)? 1: 0);
	if (Reg <3) {
		PRG[Reg] =Val;
		Sync();
	}
}

BOOL	MAPINT	Load (void) {
	WRAMAddressMask =(ROM->INES_Version <2)? 0x1FFF: ((64 << ((ROM->INES2_PRGRAM &0xF0)? ROM->INES2_PRGRAM >>4: ROM->INES2_PRGRAM &0x0F)) -1);
	iNES_SetSRAM();
	if (ROM->INES2_SubMapper ==1)
		MapperInfo_210.Description = _T("Namco N175"); 
	else if (ROM->INES2_SubMapper ==2)
		MapperInfo_210.Description = _T("Namco N340"); 
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		for (int i =0; i <4; i++) {
			PRG[i] =0xFC |i;
			CHR[i |0] = i;
			CHR[i |4] = i |4;
		}
		WRAMWriteEnable =0;
	}
	if (ROM->INES2_SubMapper ==1) iNES_SetMirroring();
	
	for (int i =0x6; i<=0x7; i++) {
		_ReadWRAM[i &1] =EMU->GetCPUReadHandler(i);
		_WriteWRAM[i &1] =EMU->GetCPUWriteHandler(i);
		EMU->SetCPUReadHandler(i, ReadWRAM);
		EMU->SetCPUReadHandlerDebug(i, ReadWRAM);
		EMU->SetCPUWriteHandler(i, WriteWRAM);
	}
	for (int i =0x8; i<=0xB; i++) EMU->SetCPUWriteHandler(i, WriteCHR);
	EMU->SetCPUWriteHandler(0xC, WriteWRAMEnable);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(i, WritePRG);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <3; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	SAVELOAD_BYTE(mode, offset, data, WRAMWriteEnable);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =210;
} // namespace

MapperInfo MapperInfo_210 ={
	&MapperNum,
	_T("Namco N175/N340"),
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