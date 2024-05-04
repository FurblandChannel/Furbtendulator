#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_N163.h"

namespace {
uint8_t	PRG[3], CHR[8], NT[4], ChipRAM[128];
uint16_t IRQCounter;
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync() {
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	for (int i =0; i <3; i++) EMU->SetPRG_ROM8(0x8 +i*2, PRG[i]);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, CHR[i]);
	EMU->Mirror_Custom(NT[0], NT[1], NT[2], NT[3]);	
}

int	MAPINT	ReadChipRAM (int Bank, int Addr) {
	if (Addr &0x800)
		return N163sound::Read((Bank <<12) | Addr);
	else
		return _Read4(Bank, Addr);
}

void	MAPINT	WriteChipRAM (int Bank, int Addr, int Val) {
	_Write4(Bank, Addr, Val);
	if (Addr &0x800) N163sound::Write((Bank <<12) | Addr, Val);
}

void	MAPINT	WritePRGandSound (int Bank, int Addr, int Val) {
	if (Bank ==0xF && Addr &0x800)
		N163sound::Write((Bank <<12) | Addr, Val);
	else
	if (~Addr &0x400) {
		PRG[((Bank &1) <<1) | ((Addr &0x800)? 1: 0)] =Val;
		Sync();
	}
}

void	MAPINT	WriteCHRandIRQ (int Bank, int Addr, int Val) {
	if (Addr &0x400 && ~Bank &4) {
		if (Bank &2) {
			IRQCounter =IRQCounter &0x00FF | (Val <<8);
			EMU->SetIRQ(1);
		} else
			IRQCounter =IRQCounter &0xFF00 |  Val;
	} else {
		CHR[((Bank &3) <<1) | ((Addr &0x800)? 1: 0)] =Val;
		Sync();
	}
}

void	MAPINT	WriteNT (int Bank, int Addr, int Val) {
	if (~Addr &0x400) NT[((Bank &1) <<1) | ((Addr &0x800)? 1: 0)] =Val &1;
	Sync();
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	N163sound::Load();
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0; i <3; i++) PRG[i] =i;
	for (int i =0; i <8; i++) CHR[i] =i;
	for (int i =0; i <4; i++) NT [i] =(i &2)? 1: 0;
	for (int i =0x8; i<=0xB; i++) EMU->SetCPUWriteHandler(i, WriteCHRandIRQ);
	for (int i =0xC; i<=0xD; i++) EMU->SetCPUWriteHandler(i, WriteNT);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(i, WritePRGandSound);
	_Read4 = EMU->GetCPUReadHandler(0x4);
	_Write4 = EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, ReadChipRAM);
	EMU->SetCPUWriteHandler(0x4, WriteChipRAM);
	N163sound::Reset(ResetType);
	Sync();
}

void	MAPINT	Unload (void) {
	N163sound::Unload();
}

void	MAPINT	CPUCycle (void) {
	if (IRQCounter &0x8000 && !++IRQCounter) EMU->SetIRQ(0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <3; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, NT [i]);
	SAVELOAD_WORD(mode, offset, data, IRQCounter);
	for (int i =0; i <128; i++) SAVELOAD_BYTE(mode, offset, data, ChipRAM[i]);
	offset =N163sound::SaveLoad(mode, offset, data);
	return offset;
}

int	MAPINT	MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? N163sound::Get(Cycles): 0;
}

uint16_t MapperNum =330;
} // namespace

MapperInfo MapperInfo_330 ={
	&MapperNum,
	_T("Sangokushi II bootleg"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	CPUCycle,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL
};
