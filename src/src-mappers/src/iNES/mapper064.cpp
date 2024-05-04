#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		pointer;
uint8_t		reg[16];
uint8_t		mirroring;

uint8_t		prescaler;
uint8_t		counter;
uint8_t		reloadValue;
uint8_t		reloadExtra;
bool		m2Counter;
bool		reload;
bool		enableIRQ;
bool		pa12;
uint8_t		pa12Filter;
uint8_t		raiseIRQ;

#define		chr1K     (pointer &0x20)
#define		prgInvert (pointer &0x40)
#define		chrInvert (pointer &0x80)

void	sync (void) {
	if (prgInvert) {
		EMU->SetPRG_ROM8(0xA, reg[0xF]);
		EMU->SetPRG_ROM8(0xC, reg[0x7]);
		EMU->SetPRG_ROM8(0x8, reg[0x6]);
	} else {
		EMU->SetPRG_ROM8(0x8, reg[0x6]);
		EMU->SetPRG_ROM8(0xA, reg[0x7]);
		EMU->SetPRG_ROM8(0xC, reg[0xF]);
	}
	EMU->SetPRG_ROM8(0xE, 0xFF);

	int chrXOR =chrInvert? 4: 0;
	if (chr1K) {
		EMU->SetCHR_ROM1(0x0 ^chrXOR, reg[0x0]);
		EMU->SetCHR_ROM1(0x1 ^chrXOR, reg[0x8]);
		EMU->SetCHR_ROM1(0x2 ^chrXOR, reg[0x1]);
		EMU->SetCHR_ROM1(0x3 ^chrXOR, reg[0x9]);
	} else {
		EMU->SetCHR_ROM2(0x0 ^chrXOR, reg[0x0] >>1);
		EMU->SetCHR_ROM2(0x2 ^chrXOR, reg[0x1] >>1);
	}
	EMU->SetCHR_ROM1(0x4 ^chrXOR, reg[0x2]);
	EMU->SetCHR_ROM1(0x5 ^chrXOR, reg[0x3]);
	EMU->SetCHR_ROM1(0x6 ^chrXOR, reg[0x4]);
	EMU->SetCHR_ROM1(0x7 ^chrXOR, reg[0x5]);
	if (ROM->INES_MapperNum ==158) {	// Extended mirroring via CHR A17
		if (chr1K) {
			EMU->SetCHR_NT1(0x8 ^chrXOR, reg[0x0] >>7);
			EMU->SetCHR_NT1(0x9 ^chrXOR, reg[0x8] >>7);
			EMU->SetCHR_NT1(0xA ^chrXOR, reg[0x1] >>7);
			EMU->SetCHR_NT1(0xB ^chrXOR, reg[0x9] >>7);
		} else {
			EMU->SetCHR_NT1(0x8 ^chrXOR, reg[0x0] >>7);
			EMU->SetCHR_NT1(0x9 ^chrXOR, reg[0x0] >>7);
			EMU->SetCHR_NT1(0xA ^chrXOR, reg[0x1] >>7);
			EMU->SetCHR_NT1(0xB ^chrXOR, reg[0x1] >>7);
		}
		EMU->SetCHR_NT1(0xC ^chrXOR, reg[0x2] >>7);
		EMU->SetCHR_NT1(0xD ^chrXOR, reg[0x3] >>7);
		EMU->SetCHR_NT1(0xE ^chrXOR, reg[0x4] >>7);
		EMU->SetCHR_NT1(0xF ^chrXOR, reg[0x5] >>7);
	} else {				// Normal mirroring via A000.0
		if (mirroring &1)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &1)
		reg[pointer &0xF] =val;
	else
		pointer =val;
	sync();
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	if (~addr &1) mirroring =val;
	sync();
}

void	MAPINT	writeIRQConfig (int bank, int addr, int val) {
	if (addr &1) {
		m2Counter =!!(val &1);
		prescaler =0;
		counter =0;
		reload =true;
		reloadExtra =pa12Filter? 0: 1;	// When writing to C001 while the counter has just been clocked via PA12, do not reload counter with latch +1 but just with latch
	} else
		reloadValue =val;
}

void	MAPINT	writeIRQEnable (int bank, int addr, int val) {
	enableIRQ =!!(addr &1);
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		EMU->SetIRQ(1);
		reg[0x0] =0x00; reg[0x8] =0x01; reg[0x1] =0x02; reg[0x9] =0x03;	// CHR registers 1
		reg[0x2] =0x04; reg[0x3] =0x05; reg[0x4] =0x06; reg[0x5] =0x07;	// CHR registers 2
		reg[0x6] =0x00; reg[0x7] =0x01; reg[0xF] =0xFE;			// PRG registers
		mirroring =0;
		
		prescaler =0;
		counter =0;
		reloadValue =0;
		reloadExtra =0;
		m2Counter =false;
		reload =false;
		enableIRQ =false;
		pa12 =false;
		pa12Filter =0;
		raiseIRQ =0;
	}
	sync();
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeMirroring);
	for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeIRQConfig);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIRQEnable);
}

void	clockIRQCounter (void) {
	if (!counter) {
		counter =reloadValue +(reload? reloadExtra: 0);
		if (!counter && reload && enableIRQ) raiseIRQ =1;
	} else
		if (!--counter && enableIRQ) raiseIRQ =1;
	reload =false;
}

void	MAPINT	cpuCycle (void) {
	if (raiseIRQ && !--raiseIRQ) EMU->SetIRQ(0);
	if (!(++prescaler &3) && m2Counter) clockIRQCounter();

	if (pa12) {
		if (pa12Filter ==0 && !m2Counter) clockIRQCounter();
		pa12Filter =16;
	} else
		if (pa12Filter) pa12Filter--;
		
	if (!enableIRQ) EMU->SetIRQ(1);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	pa12 =!!(addr &0x1000);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0x0; i<=0x9; i++) SAVELOAD_BYTE(stateMode, offset, data, reg[i]);
	SAVELOAD_BYTE(stateMode, offset, data, reg[0xF]);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, prescaler);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BYTE(stateMode, offset, data, reloadExtra);
	SAVELOAD_BOOL(stateMode, offset, data, m2Counter);
	SAVELOAD_BOOL(stateMode, offset, data, reload);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_BOOL(stateMode, offset, data, pa12);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BYTE(stateMode, offset, data, raiseIRQ);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =64;
uint16_t mapperNum2 =158;
} // namespace

MapperInfo MapperInfo_064 = {
	&mapperNum,
	_T("Tengen 800032"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_158 = {
	&mapperNum2,
	_T("Tengen 800037"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
