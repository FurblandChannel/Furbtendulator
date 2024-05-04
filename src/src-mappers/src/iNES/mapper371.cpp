#include "..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
bool		rom512;
bool		verticalMirroring;
bool		chr1bpp;
uint16_t	lastNTAddr;

FPPURead	readCHR;
FPPURead	readNT;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);

	if (rom512) {
		EMU->SetPRG_ROM16(0x8, prg +4);
		EMU->SetPRG_ROM16(0xC, prg +4);
	} else {
		EMU->SetPRG_ROM16(0x8, prg &3);
		EMU->SetPRG_ROM16(0xC, 3);
	}

	EMU->SetCHR_RAM8(0x0, 0);

	if (verticalMirroring)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	if (chr1bpp) {
		// Substitute A3 with NTRAM A0 (odd tile position)
		addr &=~0x08;
		addr |=(lastNTAddr &0x001)? 0x08: 0x00;
		
		// Substitute A12 with NTRAM A9 (scanline 128+)
		bank &=~0x04;
		bank |=(lastNTAddr &0x200)? 0x04: 0x00;
	}
	return readCHR(bank, addr);
}

int	MAPINT	interceptNTRead (int bank, int addr) {
	if (addr <0x3C0) lastNTAddr =addr;
	return readNT(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch(addr >>8) {
		case 0:	prg =prg &0x10 | val &0x0F;
			rom512 =(val &0x70) ==0x50;
			chr1bpp =!!(val &0x80);
			sync();
			break;
		case 1:	prg =prg &0x0F | (val &1? 0x10: 0x00);
			verticalMirroring =!!(val &2);
			sync();
	}
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		rom512 =false;
		verticalMirroring =false;
		chr1bpp =false;
		lastNTAddr =0;
	}
	sync();

	EMU->SetCPUWriteHandler(0x5, writeReg);
	
	readCHR =EMU->GetPPUReadHandler(0x0);
	readNT  =EMU->GetPPUReadHandler(0x8);
	for (int i =0x0; i<=0x7; i++) {
		EMU->SetPPUReadHandler(i, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(i, readCHR);
	}
	for (int i =0x8; i<=0xF; i++) {
		EMU->SetPPUReadHandler(i, interceptNTRead);
		EMU->SetPPUReadHandlerDebug(i, readNT);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BOOL(stateMode, offset, data, rom512);
	SAVELOAD_BOOL(stateMode, offset, data, verticalMirroring);
	SAVELOAD_BOOL(stateMode, offset, data, chr1bpp);
	SAVELOAD_WORD(stateMode, offset, data, lastNTAddr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =371;
} // namespace

MapperInfo MapperInfo_371 ={
	&MapperNum,
	_T("东达 PEC-586 (Spanish)"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
