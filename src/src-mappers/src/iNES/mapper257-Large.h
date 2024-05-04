#include "..\DLL\d_iNES.h"

namespace DongdaLarge {
uint8_t	Mode, Index;
uint16_t LastNTAddr;
FCPURead _ReadCart;
FPPURead _ReadCHR, _ReadNT;

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, Mode &7);
	if (~Mode &0x10 && Mode &0x40) EMU->SetPRG_ROM8(0x8, 0x20 | (Mode &0xF) | ((Mode &0x20)? 0x10: 0x00));
	if ((Mode &0x18) ==0x18)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	EMU->SetCHR_RAM8(0x0, 0);
}

int	MAPINT	ReadCHR (int Bank, int Addr) {
	if (Mode &0x80) {
		// If the 1bpp mode is active, convert 1bpp to the normal 2bpp format.
		
		// Substitute A3 with NTRAM A0 (odd tile position)
		Addr &=~0x08;
		Addr |=(LastNTAddr &0x001)? 0x08: 0x00;
		
		// Substitute A12 with NTRAM A9 (scanline 128+)
		Bank &=~0x04;
		Bank |=(LastNTAddr &0x200)? 0x04: 0x00;
	}
	return _ReadCHR(Bank, Addr);
}

int	MAPINT	ReadNT (int Bank, int Addr) {
	if (Addr <0x3C0) LastNTAddr =Addr;
	return _ReadNT(Bank, Addr);
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	switch(Addr &0x700) {
		case 0x000: Mode =Val; break;
		case 0x400: Index =Val; break;
	}
	Sync();
}

int	MAPINT	Read8F (int Bank, int Addr) {
	Addr |=Bank <<12;
	if (Mode &0x10 || (Mode &0x40 && Bank <0xA))
		return _ReadCart(Bank, Addr &0xFFF);
	else
		return ROM->PRGROMData[0x41C00 | ((Addr <<3) &0x3E000) | (Addr &0x3FF)];
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	_ReadCart =EMU->GetCPUReadHandler(0x8);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUReadHandler(i, Read8F);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUReadHandlerDebug(i, Read8F);
	EMU->SetCPUWriteHandler(0x5, Write5);
	
	_ReadCHR =EMU->GetPPUReadHandler(0x0);
	_ReadNT  =EMU->GetPPUReadHandler(0x8);
	for (int i =0x0; i<=0x7; i++) {
		EMU->SetPPUReadHandler(i, ReadCHR);
		EMU->SetPPUReadHandlerDebug(i, _ReadCHR);
	}
	for (int i =0x8; i<=0xF; i++) {
		EMU->SetPPUReadHandler(i, ReadNT);
		EMU->SetPPUReadHandlerDebug(i, _ReadNT);
	}
	if (ResetType == RESET_HARD) {
		Index =0;
		Mode =0;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Mode);
	SAVELOAD_BYTE(mode, offset, data, Index);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =257;

MapperInfo MapperInfo_DongdaLarge ={
	&MapperNum,
	_T("东达 PEC-586 (Large)"),
	COMPAT_FULL,
	Load,
	Reset,
	::Unload,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
} // namespace
