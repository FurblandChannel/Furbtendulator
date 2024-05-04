#include "..\DLL\d_iNES.h"

namespace {
uint8_t	PRG[2], CHR[8], MirrS, MirrHV, IRQEnabled, IRQCounter;

void	Sync (void) {
	EMU->SetPRG_ROM8(0x8, PRG[0]);
	EMU->SetPRG_ROM8(0xA, PRG[1]);
	EMU->SetPRG_ROM16(0xC, -1);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, CHR[i]);
	switch(MirrS) {
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
		default: if (MirrHV)
				EMU->Mirror_H();
			else
				EMU->Mirror_V();
			break;
	}
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	switch (Bank &0x0C) {
		case 0x8:	if ((Addr &0xC)==0x4) MirrS = Val &3; else
				if ((Addr &0xC)==0xC) EMU->SetIRQ(0);
				break;
		case 0xC:	if ((Addr&0xC) ==0x4) EMU->SetIRQ(1); else
				if ((Addr&0xC) ==0x8) IRQEnabled =1; else
				if ((Addr&0xC) ==0xC) { IRQEnabled =0; IRQCounter =0; EMU->SetIRQ(1); }
				break;
	}
	switch(Bank) {
		case 0x08:	PRG[0] =Val; break;
		case 0x09:	MirrHV =Val &1; break;
		case 0x0A:	PRG[1] =Val; break;
		case 0x0F:	break;
		default:	int Reg = ((Bank-0x0B)<<1) + ((Addr &2)>>1);
				if (~Addr &1)
					CHR[Reg] = CHR[Reg] &~0x0F | (Val &0x0F);
				else
					CHR[Reg] = CHR[Reg] &~0xF0 | (Val <<4);
				break;
	}
	Sync();
}

int	LastAddr =0;
void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0; i <8; i++) EMU->SetCPUWriteHandler(i +8, Write);
	if (ResetType == RESET_HARD) {
		for (int i =0; i <8; i++) CHR[i] =0;
		PRG[0] =PRG[1] =MirrS =MirrHV =IRQEnabled =IRQCounter =0;
	}
	LastAddr =0;
	Sync();
}

void	MAPINT	PPUCycle (int Addr, int Scanline, int Cycle, int IsRendering) {
	if (LastAddr &0x2000 && ~Addr &0x2000 && IRQEnabled && ++IRQCounter==84) {
		IRQCounter =0;
		EMU->SetIRQ(0);
	}
	LastAddr =Addr;
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <2; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	SAVELOAD_BYTE(mode, offset, data, MirrS);
	SAVELOAD_BYTE(mode, offset, data, MirrHV);
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_BYTE(mode, offset, data, IRQCounter);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum = 272;
} // namespace

MapperInfo MapperInfo_272 = {
	&MapperNum,
	_T("J-2012-II"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	PPUCycle,
	SaveLoad,
	NULL,
	NULL
};
