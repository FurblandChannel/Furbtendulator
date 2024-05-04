#include	"..\..\DLL\d_iNES.h" 

namespace {
uint8_t		prg;
bool		irq;
uint16_t	counter;
uint8_t		outer;

void	sync (void) {
	if (outer &8) {
		if (outer &0x10)
			EMU->SetPRG_ROM32(0x8, 2 | outer >>6);
		else {
			EMU->SetPRG_ROM16(0x8, 4 | outer >>5);
			EMU->SetPRG_ROM16(0xC, 4 | outer >>5);
		}
	} else {
		EMU->SetPRG_ROM8(0x6, 6);
		EMU->SetPRG_ROM8(0x8, 4);
		EMU->SetPRG_ROM8(0xA, 5);
		EMU->SetPRG_ROM8(0xC, prg &7);
		EMU->SetPRG_ROM8(0xE, 7);
	}
	iNES_SetCHR_Auto8(0x0, outer >>1);
	if (outer &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	irq =!!(bank &2);
}

void	MAPINT	writeOuter (int bank, int addr, int val) {
	outer =addr &0xFF;
	sync();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prg =outer =0;
	sync();
	irq =false;
	counter =0;
	EMU->SetIRQ(1);
	
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeIRQ);
	if (ROM->INES2_SubMapper ==1) for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeOuter);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (irq)
		EMU->SetIRQ(++counter &0x1000? 0: 1);
	else {
		EMU->SetIRQ(1);
		counter =0;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BOOL(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, outer);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =40;
} // namespace

MapperInfo MapperInfo_040 ={
	&mapperNum,
	_T("NTDEC 2722"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};