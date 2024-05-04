#include "..\DLL\d_iNES.h"

namespace {
uint8_t	Mode, Index;
uint16_t LastNTAddr;
FPPURead _ReadCHR, _ReadNT;
uint8_t prgSW;

FCPURead	readCart;

static uint8_t ProtLUT[16] = {
	0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, Mode >>5 &3);
	if (prgSW &2)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
	EMU->SetCHR_RAM8(0x0, 0);
}

int	MAPINT	readPRG (int bank, int addr) {
	if (Mode &1) {
		// Start at C400
		uint16_t encryptedAddress =bank <<12 &0x7000 |addr;
		//atic const uint8_t addressOrder [15] ={  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14 };
		static const uint8_t addressOrder [15] ={ 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 12, 13, 14 };
		uint16_t decryptedAddress =0;
		for (uint8_t bit =0; bit <15; bit++) decryptedAddress |=(encryptedAddress >>addressOrder[bit] &1) <<bit;
		
		addr =decryptedAddress &0xFFF;
		bank =decryptedAddress >>12 |8;
		uint8_t encryptedData =readCart(bank, addr);
		
		static const uint8_t dataOrder [15] ={ 7, 6, 5, 4, 3, 2, 1, 0 };
		uint8_t decryptedData =0;
		for (uint8_t bit =0; bit <8; bit++) decryptedData |=(encryptedData >>dataOrder[bit] &1) <<bit;
		return decryptedData;
	}
	return readCart(bank, addr);
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

int	blah;
int	MAPINT	Read5 (int Bank, int Addr) {
	switch(Addr &0x700) {
		case 0x300: blah ^=4; return blah;
		case 0x500: return (*EMU->OpenBus &0xD8) | (ProtLUT[Index >>4]);
		default:    return *EMU->OpenBus;
	}
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	switch(Addr &0x700) {
		case 0x000:
			//if (Mode !=Val) EMU->DbgOut(L"%02X", Val);
			Mode =Val;
			break;
		case 0x100:	prgSW =Val; Sync(); break;
		case 0x400: Index =Val; break;
	}
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->SetCPUReadHandler(0x5, Read5);
	EMU->SetCPUReadHandlerDebug(0x5, Read5);
	EMU->SetCPUWriteHandler(0x5, Write5);
	
	readCart =EMU->GetCPUReadHandler(0x8);
	_ReadCHR =EMU->GetPPUReadHandler(0x0);
	_ReadNT  =EMU->GetPPUReadHandler(0x8);
	for (int i =0x0; i<=0x7; i++) {
		EMU->SetPPUReadHandler(i, ReadCHR);
		EMU->SetPPUReadHandlerDebug(i, _ReadCHR);
	}
	for (int i =0x8; i<=0xF; i++) {
		EMU->SetPPUReadHandler(i, ReadNT);
		EMU->SetPPUReadHandlerDebug(i, _ReadNT);
		
		EMU->SetCPUReadHandler(i, readPRG);
		EMU->SetCPUReadHandlerDebug(i, readPRG);
	}
	if (ResetType == RESET_HARD) {
		Index =0;
		Mode =0x0E;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Mode);
	SAVELOAD_BYTE(mode, offset, data, Index);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =440;
} // namespace

MapperInfo MapperInfo_440 ={
	&MapperNum,
	_T("Sonic REC-9388"),
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
