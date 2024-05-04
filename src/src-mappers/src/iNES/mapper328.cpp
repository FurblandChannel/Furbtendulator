#include	"..\DLL\d_iNES.h"

#include <stdio.h>
#include <stdlib.h>

namespace {
FCPURead _ReadCart;

int	MAPINT	Read (int Bank, int Addr) {
	Addr |=Bank <<12;
	if (Addr >=0xCE80 && Addr <0xCF00 || Addr >=0xFE80 && Addr <0xFF00)
		return 0xF2 | (rand() & 0x0D);
	else
		return _ReadCart(Bank, Addr &0xFFF);
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	_ReadCart =EMU->GetCPUReadHandler(0x8);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUReadHandler(i, Read);
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, 0);
	
}

uint16_t MapperNum =328;
} // namespace

MapperInfo MapperInfo_328 ={
	&MapperNum,
	_T("RT-01"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};