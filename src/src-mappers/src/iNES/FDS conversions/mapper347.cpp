#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t	Reg0, Reg1, Mirroring;

void	Sync (void) {
	EMU->SetCHR_RAM8(0x0, 0);
	if (Mirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();	
}

int	MAPINT	Read (int Bank, int Addr) {
	Addr |=Bank <<12;
	if (Addr >=0x6000 && Addr <=0x6BFF) return ROM->PRGRAMData[Addr -0x6000 +0x00000]; else
	if (Addr >=0x6C00 && Addr <=0x6FFF) return ROM->PRGROMData[Addr -0x6000 +0x01000*Reg1 +0x00000]; else
	if (Addr >=0x7000 && Addr <=0x7FFF) return ROM->PRGROMData[Addr -0x7000 +0x01000*Reg0 +0x10000]; else
	if (Addr >=0xB800 && Addr <=0xBFFF) return ROM->PRGRAMData[Addr -0xB800 +0x00C00]; else
	if (Addr >=0xC000 && Addr <=0xCBFF) return ROM->PRGROMData[Addr -0xC000 +0x01000*Reg1 +0x00000]; else
	if (Addr >=0xCC00 && Addr <=0xD7FF) return ROM->PRGRAMData[Addr -0xCC00 +0x01400]; else 
	if (Addr >=0x8000)                  return ROM->PRGROMData[Addr -0x8000 +0x18000]; else
	return *EMU->OpenBus;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Addr |=Bank <<12;
	if (Addr >=0x6000 && Addr <=0x6BFF) ROM->PRGRAMData[Addr -0x6000 +0x0000] =Val; else
	if (Addr >=0xB800 && Addr <=0xBFFF) ROM->PRGRAMData[Addr -0xB800 +0x0C00] =Val; else
	if (Addr >=0xCC00 && Addr <=0xD7FF) ROM->PRGRAMData[Addr -0xCC00 +0x1400] =Val; else
	if (Addr >=0x8000 && Addr <=0x8FFF) { Reg0 =Addr & 7; Mirroring =Addr &8; Sync(); } else
	if (Addr >=0x9000 && Addr <=0x9FFF) { Reg1 =Addr &15; Sync(); }
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	Reg0 =Reg1 =0xFF;
	iNES_SetMirroring();
	for (int i =0x6; i<=0xF; i++) {
		EMU->SetCPUReadHandler(i, Read);
		EMU->SetCPUReadHandlerDebug(i, Read);
		EMU->SetCPUWriteHandler(i, Write);
	}
	
	FDSsound::ResetBootleg(ResetType);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg0);
	SAVELOAD_BYTE(mode, offset, data, Reg1);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	offset = FDSsound::SaveLoad(mode, offset, data);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}


uint16_t MapperNum =347;
} // namespace

MapperInfo MapperInfo_347 ={
	&MapperNum,
	_T("Kaiser KS-7030"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL
};
