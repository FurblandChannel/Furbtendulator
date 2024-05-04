#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_N163.h"

#include <stdio.h>
#include <stdlib.h>

namespace {
uint8_t		prg[4];
uint8_t		chr[12];
uint8_t		mirroring;
uint8_t		wramProtect;
int16_t		counter;

bool		soundEnabled;
uint8_t		volume;

FCPURead	readAPU;
FCPUWrite	writeAPU;
FCPUWrite	writeWRAM;

void	sync (void) {
	// CPU Address Space
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0; i <4; i++) EMU->SetPRG_ROM8(0x8 +i*2, prg[i] &0x3F);
	
	// PPU Address Space
	for (int i =0; i <12; i++) {
		bool forceVROM =false;
		if (i <4 && prg[1] &0x40) forceVROM =true; else
		if (i <8 && prg[1] &0x80) forceVROM =true;
		
		if (chr[i] <0xE0 || forceVROM)
			EMU->SetCHR_ROM1(i, chr[i]);
		else
			EMU->SetCHR_NT1(i, chr[i] &(CART_VRAM? 3: 1));
	}
}

int	MAPINT	readChipRAM (int bank, int addr) {
	if (addr &0x800)
		return N163sound::Read(bank <<12 | addr);
	else
		return readAPU(bank, addr);
}

void	MAPINT	writeChipRAM (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x800) N163sound::Write(bank <<12 | addr, val);
}

int	MAPINT	readCounter (int bank, int addr) {
	return addr &0x800? counter >>8: counter &0xFF;
}

void	MAPINT	writeCounter (int bank, int addr, int val) {
	EMU->SetIRQ(1);
	if (addr &0x800)
		counter =counter &0x00FF | val <<8;
	else
		counter =counter &0xFF00 | val;
}

void	MAPINT	writeProtectedWRAM (int bank, int addr, int val) {
	uint8_t shift =bank <<1 &6 | addr >>11 &1;
	uint8_t mask =0xF0 | 1 <<shift;
	if ((wramProtect &mask) ==0x40) writeWRAM(bank, addr, val);
}

void	MAPINT	writeCHR (int bank, int addr, int val) {	
	int reg =bank <<1 &0xE | addr >>11 &1;
	chr[reg] =val;
	sync();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	int reg =bank <<1 &2 | addr >>11 &1;
	if (reg <3) {
		prg[reg] =val;
		if (reg ==0) soundEnabled =!(val &0x40);
		sync();
	} else {
		wramProtect =val;
		N163sound::Write((bank <<12) | addr, val);
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	N163sound::Load();
	ROM->ChipRAMSize =128;
	ROM->ChipRAMData =N163sound::ChipRAM;
	// 	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	static const uint8_t submapperVolume[16] = {
		34, 34,  0, 34, 59, 68, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34
	};
	volume =submapperVolume[ROM->INES2_SubMapper];	
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	N163sound::Reset(resetType);
	if (resetType ==RESET_HARD) {
		for (int i =0; i <4; i++) {
			prg[i] = i |0xFC;
			chr[i |0] = i;
			chr[i |4] = i       |0x04;
			chr[i |8] = i &0x01 |0xE0;
		}
		counter =0;		
		wramProtect =0xFF;
	}
	EMU->SetIRQ(1);
	sync();	

	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	writeWRAM =EMU->GetCPUWriteHandler(0x6);
	EMU->SetCPUReadHandler(0x4, readChipRAM);
	EMU->SetCPUWriteHandler(0x4, writeChipRAM);
	EMU->SetCPUReadHandler(0x5, readCounter);
	EMU->SetCPUWriteHandler(0x5, writeCounter);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUWriteHandler(i, writeProtectedWRAM);
	for (int i =0x8; i<=0xD; i++) EMU->SetCPUWriteHandler(i, writeCHR);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writePRG);
}

void	MAPINT	unload (void) {
	N163sound::Unload();
}

void	MAPINT	cpuCycle (void) {
	if (counter <-1 && ++counter ==-1) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, wramProtect);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BOOL(stateMode, offset, data, soundEnabled);
	offset =N163sound::SaveLoad(stateMode, offset, data);	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSound (int cycles) {
	return soundEnabled? N163sound::Get(cycles) *volume /40: 0;
}

uint16_t MapperNum =19;
uint16_t MapperNum2 =532;
} // namespace

MapperInfo MapperInfo_019 ={
	&MapperNum,
	_T("Namco N129/N163"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSound,
	NULL
};
MapperInfo MapperInfo_532 ={
	&MapperNum2,
	_T("CHINA_ER_SAN2"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSound,
	NULL
};