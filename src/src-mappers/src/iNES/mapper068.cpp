#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		chr[4];
uint8_t		nt[2];
uint8_t		ntConfig;
uint8_t		prg;

uint32_t	timer;
bool		externalAccess;
FCPUWrite	writeWRAM;

#define	externalROM !(prg &0x08)
#define	enableWRAM  !!(prg &0x10)
#define enableVROM  !!(ntConfig &0x10)
#define	mirroring   (ntConfig &0x03)

void	sync (void) {
	if (enableWRAM)
		EMU->SetPRG_RAM8(0x6, 0);
	else
		for (int bank =0x6; bank<=0x7; bank++) EMU->SetPRG_OB4(bank);
	
	if (ROM->INES2_SubMapper ==1) {
		EMU->SetPRG_ROM16(0xC, 0x07);
		if (externalROM) {
			if (externalAccess)
				EMU->SetPRG_ROM16(0x8, 0x8);
			else
				for (int bank =0x8; bank<=0xB; bank++) EMU->SetPRG_OB4(bank);
		} else
			EMU->SetPRG_ROM16(0x8, prg &0x07);
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, 0xFF);
	}

	for (int bank =0; bank <4; bank++) EMU->SetCHR_ROM2(bank *2, chr[bank]);
	
	if (enableVROM) for (int bank =0x8; bank<=0xF; bank++) switch (mirroring) {
		case 0:	EMU->SetCHR_ROM1(bank, nt[bank &1      ] |0x80); break;
		case 1:	EMU->SetCHR_ROM1(bank, nt[(bank >>1) &1] |0x80); break;
		case 2:	EMU->SetCHR_ROM1(bank, nt[0            ] |0x80); break;
		case 3:	EMU->SetCHR_ROM1(bank, nt[1            ] |0x80); break;
	} else switch (mirroring) {
		case 0: EMU->Mirror_V (); break;
		case 1: EMU->Mirror_H (); break;
		case 2: EMU->Mirror_S0(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

void	MAPINT	interceptWRAMWrite (int bank, int addr, int val) {
	if (enableWRAM)
		writeWRAM(bank, addr, val);
	else {
		externalAccess =true;
		timer =107520;
		sync();
	}
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[bank &3] =val;
	sync();
}

void	MAPINT	writeNT (int bank, int addr, int val) {
	nt[bank &1] =val;
	sync();
}

void	MAPINT	writeNTConfig (int bank, int addr, int val) {
	ntConfig =val;
	sync();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (int bank =0; bank <4; bank++) chr[bank] =bank;
		for (int bank =0; bank <2; bank++) nt [bank] =bank;
		ntConfig =0x00;
		prg =0x00;
		externalAccess = false;
		timer =0;
	}
	sync();

	if (ROM->INES2_SubMapper==1) {
		writeWRAM =EMU->GetCPUWriteHandler(0x6);
		for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, interceptWRAMWrite);
	}
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeNT);
	EMU->SetCPUWriteHandler(0xE, writeNTConfig);
	EMU->SetCPUWriteHandler(0xF, writePRG);
}

void	MAPINT	cpuCycle (void) {
	if (timer && !--timer) {
		externalAccess =false;
		sync();
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: nt) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, ntConfig);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (ROM->INES2_SubMapper ==1) {
		SAVELOAD_BOOL(stateMode, offset, data, externalAccess);
		SAVELOAD_LONG(stateMode, offset, data, timer);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =68;
} // namespace

MapperInfo MapperInfo_068 ={
	&mapperNum,
	_T("Sunsoft-4"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};