#include "..\DLL\d_iNES.h"

namespace {
uint8_t	PRG[2], CHR[8], HorizontalMirroring, IRQEnabled, PrescalerMask, Prescaler, IRQCounter;

void	Sync (void) {
	EMU->SetPRG_ROM8(0x8, PRG[0]);
	EMU->SetPRG_ROM8(0xA, PRG[1]);
	EMU->SetPRG_ROM16(0xC, -1);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, CHR[i]);
	if (HorizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <2; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	SAVELOAD_BYTE(mode, offset, data, HorizontalMirroring);
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_BYTE(mode, offset, data, IRQCounter);
	SAVELOAD_BYTE(mode, offset, data, Prescaler);
	SAVELOAD_BYTE(mode, offset, data, PrescalerMask);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	PRG[(Bank ==8)? 0: 1] =Val;
	Sync();
}

void	MAPINT	WriteMirroring (int Bank, int Addr, int Val) {
	HorizontalMirroring =Val &1;
	Sync();
}

void	MAPINT	WriteCHR (int Bank, int Addr, int Val) {
	int Reg = ((Bank -0xB) <<1) | ((Addr &8) >>3);
	if (~Addr &4)
		CHR[Reg] = CHR[Reg] &~0x0F | (Val &0x0F);
	else
		CHR[Reg] = CHR[Reg] &~0xF0 | (Val <<4);
	Sync();
}

void	MAPINT	WriteIRQ (int Bank, int Addr, int Val) {
	switch (Addr &8) {
		case 0:	IRQCounter =Val;
			EMU->SetIRQ(1);
			break;
		case 8:	if (Val &1)
				IRQEnabled =1;
			else {
				IRQEnabled =0;
				Prescaler =0;
				PrescalerMask =0x7F;
				EMU->SetIRQ(1);
			}
			break;
	}
}

void	MAPINT	CPUCycle (void) {
	if (IRQEnabled && (++Prescaler &PrescalerMask)==0x00) {
		PrescalerMask =0xFF;
		if (!++IRQCounter)
			EMU->SetIRQ(0);
		else
			EMU->SetIRQ(1);
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0x8; i <=0xA; i+=2) EMU->SetCPUWriteHandler(i, WritePRG);
	for (int i =0xB; i <=0xE; i+=1) EMU->SetCPUWriteHandler(i, WriteCHR);
	EMU->SetCPUWriteHandler(0x9, WriteMirroring);
	EMU->SetCPUWriteHandler(0xF, WriteIRQ);
	if (ResetType == RESET_HARD) {
		for (int i =0; i <8; i++) CHR[i] =0;
		PRG[0] =PRG[1] =HorizontalMirroring =IRQEnabled =IRQCounter =Prescaler =PrescalerMask =0;
	}
	Sync();
}

uint16_t MapperNum = 273;
} // namespace

MapperInfo MapperInfo_273 = {
	&MapperNum,
	_T("Bootleg Gremlins 2"),
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
