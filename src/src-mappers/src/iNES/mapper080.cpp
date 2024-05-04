#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg[3];
uint8_t		chr[6];
uint8_t		mirroring;

void	sync (void) {
	EMU->SetPRG_ROM8(0x8, prg[0]);
	EMU->SetPRG_ROM8(0xA, prg[1]);
	EMU->SetPRG_ROM8(0xC, prg[2]);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	
	EMU->SetCHR_ROM2(0, chr[0] >>1);
	EMU->SetCHR_ROM2(2, chr[1] >>1);
	EMU->SetCHR_ROM1(4, chr[2]);
	EMU->SetCHR_ROM1(5, chr[3]);
	EMU->SetCHR_ROM1(6, chr[4]);
	EMU->SetCHR_ROM1(7, chr[5]);
	
	if (ROM->INES_MapperNum ==207)
		EMU->Mirror_Custom(chr[0] >>7, chr[0] >>7, chr[1] >>7, chr[1] >>7);
	else
	if (mirroring &1)
		EMU->Mirror_V();
	else	
		EMU->Mirror_H();
}

int 	MAPINT	readRAM (int bank, int addr) {
	addr |=bank <<12;
	if (addr >=0x7F00)
		return ROM->PRGRAMData[addr &0x7F];
	else
		return *EMU->OpenBus;
}

void	MAPINT	writeRegRAM (int bank, int addr, int val) {
	addr |=bank <<12;
	switch(addr) {
		case 0x7EF0: case 0x7EF1: case 0x7EF2: case 0x7EF3: case 0x7EF4: case 0x7EF5:
			chr[addr &7] =val;
			sync();
			break;
		case 0x7EF6:
			mirroring =val;
			sync();
			break;
		case 0x7EFA: case 0x7EFB: case 0x7EFC: case 0x7EFD: case 0x7EFE: case 0x7EFF:
			prg[(addr -0x7EFA) >>1] =val;
			sync();
			break;
		default:
			if (addr >=0x7F00)
				ROM->PRGRAMData[addr &0x7F] =val;
			break;
	}
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_Version <2)
		EMU->Set_SRAMSize(ROM->INES_Flags &0x02? 128: 0);
	else
		iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[0] =0x00;
		prg[1] =0x01;
		prg[2] =0xFE;
		for (auto& c: chr) c =0;
		mirroring =0;
	}
	sync();
	
	for (int bank =0x6; bank<=0x7; bank++) {
		EMU->SetCPUReadHandler(bank, readRAM);
		EMU->SetCPUReadHandlerDebug(bank, readRAM);
		EMU->SetCPUWriteHandler(bank, writeRegRAM);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum080 =80;
uint16_t mapperNum207 =207;
} // namespace

MapperInfo MapperInfo_080 ={
	&mapperNum080,
	_T("Taito P3-33/34/36"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_207 ={
	&mapperNum207,
	_T("Taito Ashura"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};