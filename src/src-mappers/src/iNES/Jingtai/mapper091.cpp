#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prg[2];
uint8_t		chr[4];
uint8_t		outerBank;
bool		horizontalMirroring;
bool		irqEnabled;
uint8_t		m2Prescaler;
uint8_t		pa12Counter;
int16_t		previousPA12;
int16_t		m2Counter;

void	sync (void) {
	EMU->SetPRG_ROM8 (0x8, prg[0] &0xF | outerBank <<3 &~0xF);
	EMU->SetPRG_ROM8 (0xA, prg[1] &0xF | outerBank <<3 &~0xF);
	EMU->SetPRG_ROM8 (0xC,         0xE | outerBank <<3 &~0xF);
	EMU->SetPRG_ROM8 (0xE,         0xF | outerBank <<3 &~0xF);
	
	EMU->SetCHR_ROM2 (0x0, chr[0] | outerBank <<8 &0x100);
	EMU->SetCHR_ROM2 (0x2, chr[1] | outerBank <<8 &0x100);
	EMU->SetCHR_ROM2 (0x4, chr[2] | outerBank <<8 &0x100);
	EMU->SetCHR_ROM2 (0x6, chr[3] | outerBank <<8 &0x100);
	
	if (ROM->INES2_SubMapper ==0)
		iNES_SetMirroring();
	else
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &3] =val;
	sync();
}

void	MAPINT	writeCHR_m2 (int bank, int addr, int val) {
	switch (addr &7) {
		case 0: case 1: case 2: case 3:
			chr[addr &3] =val;
			break;
		case 4:	horizontalMirroring =true;
			break;
		case 5:	horizontalMirroring =false;
			break;
		case 6:	m2Counter =m2Counter &0xFF00 |val;
			break;
		case 7:	m2Counter =m2Counter &0x00FF |val <<8;
			break;
	}
	sync();
}

void	MAPINT	writePRG_IRQ (int bank, int addr, int val) {
	switch (addr &3) {
		case 0:	case 1:
			prg[addr &1] =val;
			break;
		case 2:	irqEnabled =false;
			pa12Counter =0;
			EMU->SetIRQ(1);
			break;
		case 3:	irqEnabled =true;
			m2Prescaler =3;
			break;
	}
	sync();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper ==1) {
		MapperInfo_091.PPUCycle =NULL;
		MapperInfo_091.Description =_T("EJ-006-1");
	} else {
		MapperInfo_091.CPUCycle =NULL;
		MapperInfo_091.Description =ROM->PRGROMSize ==512*1024? _T("晶太 YY840208C"): ROM->CHRROMSize ==512*1024? _T("晶太 JY830619C"): _T("晶太 JY830623C");
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (int bank =0; bank <2; bank++) prg[bank] =bank;
		for (int bank =0; bank <4; bank++) chr[bank] =bank;
		outerBank =0;
		horizontalMirroring =false;
		pa12Counter =0;
		m2Counter =0;
		previousPA12 =0;
		irqEnabled =false;
		EMU->SetIRQ(1);
	}
	sync();
	EMU->SetCPUWriteHandler(0x6, ROM->INES2_SubMapper ==1? writeCHR_m2: writeCHR);
	EMU->SetCPUWriteHandler(0x7, writePRG_IRQ);
	EMU->SetCPUWriteHandler(0x8, writeOuterBank);
	EMU->SetCPUWriteHandler(0x9, writeOuterBank);
}

void	MAPINT	cpuCycle (void) {
	if ((++m2Prescaler &3) ==0) {
		m2Counter -=5;
		if (m2Counter <=0 && irqEnabled) EMU->SetIRQ(0);
	}
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000 && (addr &0x1000) !=previousPA12 && irqEnabled && ++pa12Counter >=64) EMU->SetIRQ(0);
	previousPA12 =addr &0x1000;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i = 0; i <2; i++) SAVELOAD_BYTE(stateMode, offset, data, prg[i]);
	for (int i = 0; i <4; i++) SAVELOAD_BYTE(stateMode, offset, data, chr[i]);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	if (ROM->INES2_SubMapper ==1) {
		SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
		SAVELOAD_BYTE(stateMode, offset, data, m2Prescaler);
		SAVELOAD_WORD(stateMode, offset, data, m2Counter);
	} else {
		SAVELOAD_BYTE(stateMode, offset, data, pa12Counter);
		SAVELOAD_WORD(stateMode, offset, data, previousPA12);
	}
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =91;
} // namespace

MapperInfo MapperInfo_091 ={
	&mapperNum,
	_T("EJ-006-1/晶太 YY830624C/JY830848C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};