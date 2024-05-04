#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg[16];
uint8_t		latch[8];
uint16_t	counter;

struct X1_017 { enum {
	chr0, chr2, chr4, chr5, chr6, chr7,
	mode,
	protectA, protectB, protectC,
	prg8, prgA, prgC,
	latch, irq, ack
};};
struct Mode { enum {
	verticalMirroring =0x01,
	chrFlip =0x02
};};
struct IRQ { enum {
	counting =0x01,
	enabled =0x02,
	source =0x04
};};

uint8_t prgBits (uint8_t val) {
	if (ROM->INES_MapperNum ==552)
		return val >>5 &0x01 | val >>3 &0x02 | val >>1 &0x04 | val <<1 &0x08 | val <<3 &0x10 | val <<5 &0x20;
	else	// mapper 082
		return val >>2;
}

void	sync (void) {
	EMU->SetPRG_ROM8(0x8, prgBits(reg[X1_017::prg8]));
	EMU->SetPRG_ROM8(0xA, prgBits(reg[X1_017::prgA]));
	EMU->SetPRG_ROM8(0xC, prgBits(reg[X1_017::prgC]));
	EMU->SetPRG_ROM8(0xE, 0xFF);
	
	int chrFlip =reg[X1_017::mode] &Mode::chrFlip? 4: 0;
	EMU->SetCHR_ROM2(0 ^chrFlip, reg[X1_017::chr0] >>1);
	EMU->SetCHR_ROM2(2 ^chrFlip, reg[X1_017::chr2] >>1);
	EMU->SetCHR_ROM1(4 ^chrFlip, reg[X1_017::chr4]);
	EMU->SetCHR_ROM1(5 ^chrFlip, reg[X1_017::chr5]);
	EMU->SetCHR_ROM1(6 ^chrFlip, reg[X1_017::chr6]);
	EMU->SetCHR_ROM1(7 ^chrFlip, reg[X1_017::chr7]);
	
	if (reg[X1_017::mode] &Mode::verticalMirroring)
		EMU->Mirror_V();
	else	
		EMU->Mirror_H();
}

int 	MAPINT	readRAMDebug (int bank, int addr) {
	addr |=bank <<12;
	if (addr >=0x6000 && addr <=0x67FF && reg[X1_017::protectA] ==0xCA ||
	    addr >=0x6800 && addr <=0x6FFF && reg[X1_017::protectB] ==0x69 || 
	    addr >=0x7000 && addr <=0x73FF && reg[X1_017::protectC] ==0x84) {
		if (addr &0x3FF)
			return ROM->PRGRAMData[addr &0x1FFF];
		else
			return latch[addr >>10 &7];
	} else
		return *EMU->OpenBus;
}
int 	MAPINT	readRAM (int bank, int addr) {
	addr |=bank <<12;
	if (addr >=0x6000 && addr <=0x67FF && reg[X1_017::protectA] ==0xCA ||
	    addr >=0x6800 && addr <=0x6FFF && reg[X1_017::protectB] ==0x69 || 
	    addr >=0x7000 && addr <=0x73FF && reg[X1_017::protectC] ==0x84) {
		if (addr &0x3FF) {
			latch[addr >>10 &7] =ROM->PRGRAMData[addr &0x1FFF];
			return ROM->PRGRAMData[addr &0x1FFF];
		} else
			return latch[addr >>10 &7];
	} else
		return *EMU->OpenBus;
}

void	MAPINT	writeRegRAM (int bank, int addr, int val) {
	addr |=bank <<12;
	if (addr >=0x6000 && addr <=0x67FF && reg[X1_017::protectA] ==0xCA ||
	    addr >=0x6800 && addr <=0x6FFF && reg[X1_017::protectB] ==0x69 || 
	    addr >=0x7000 && addr <=0x73FF && reg[X1_017::protectC] ==0x84) {
		latch[addr >>10 &7] =val;
		if (ROM->PRGRAMSize >=5120) ROM->PRGRAMData[addr &0x1FFF] =val;
	} else
	if (addr >=0x7EF0 && addr <=0x7EFE) {
		reg[addr -0x7EF0] =val;
		sync();
	} else
	if (addr ==0x7EFF)
		counter =reg[X1_017::latch]? (reg[X1_017::latch] +1) *16: 1;
}

BOOL	MAPINT	load (void) {
	EMU->Set_SRAMSize(ROM->INES_Flags &0x02? 5120: 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: reg) c =0;
		for (auto& c: latch) c =0;
		reg[X1_017::prg8] =0x00;
		reg[X1_017::prgA] =0x01;
		reg[X1_017::prgC] =0xFE;
	}
	sync();
	
	for (int bank =0x6; bank<=0x7; bank++) {
		if (ROM->PRGRAMSize) {
			EMU->SetCPUReadHandler(bank, readRAM);
			EMU->SetCPUReadHandlerDebug(bank, readRAMDebug);
		}
		EMU->SetCPUWriteHandler(bank, writeRegRAM);
	}
}

void	MAPINT	cpuCycle() {
	if (reg[X1_017::irq] &IRQ::counting && ~reg[X1_017::irq] &IRQ::source) {
		if (counter) counter--;
	} else
		counter =reg[X1_017::latch]? (reg[X1_017::latch] +2) *16: 17;
	EMU->SetIRQ(reg[X1_017::irq] &IRQ::enabled && counter ==0? 0: 1);
	
	*EMU->OpenBus =0; // Pull-down registers force open bus to zero
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: latch) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum082 =82;
uint16_t mapperNum552 =552;
} // namespace

MapperInfo MapperInfo_082 ={
	&mapperNum082,
	_T("Taito P3-044 (wrong PRG order)"),
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
MapperInfo MapperInfo_552 ={
	&mapperNum552,
	_T("Taito P3-044"),
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
