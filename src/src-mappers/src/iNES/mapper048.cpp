#include	"..\DLL\d_iNES.h"

/*		IRQ?	Mirroring	Mapper
TC0190		N	$8000.6		33
TC0190+PAL16R4	N	$E000.6		48
TC0390		Y	$8000.6		33
TC0690		Y	$E000.6		48.0 (Flintstones)
TC0690		Y	$E000.6		48.1 (Jetsons)
*/

namespace {
uint8_t		reg[8];
uint8_t		mirroring;
uint8_t		counter;
uint8_t		latch;
uint8_t		lastPA12;
bool		irq;
bool		pending;
FPPURead	readCHR;

void	sync (void) {
	EMU->SetPRG_ROM8 (0x8, reg[0]);
	EMU->SetPRG_ROM8 (0xA, reg[1]);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_ROM2 (0x0, reg[2]);
	EMU->SetCHR_ROM2 (0x2, reg[3]);
	EMU->SetCHR_ROM1 (0x4, reg[4]);
	EMU->SetCHR_ROM1 (0x5, reg[5]);
	EMU->SetCHR_ROM1 (0x6, reg[6]);
	EMU->SetCHR_ROM1 (0x7, reg[7]);
	
	if ((ROM->INES_MapperNum ==48? mirroring: reg[0]) &0x40)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	interceptCHRRead (int bank, int addr) {
	if (lastPA12 &4 && ~bank &4 && counter++ ==(ROM->INES2_SubMapper ==1? 0x00: 0xFF)) pending =true;
	EMU->SetIRQ(pending && irq? 0: 1);
	lastPA12 =bank;	
	return readCHR(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[bank <<1 &4 | addr &3] =val;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	switch(addr &3) {
		case 0:	latch =val;
			break;
		case 1:	counter =latch;
			pending =false;
			break;
		case 2: irq =true;
			break;
		case 3:	irq =false;
			break;
	}
	EMU->SetIRQ(pending && irq? 0: 1);
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		reg[0] =0x00; reg[1] =0x01;
		reg[2] =0x00; reg[3] =0x01;
		reg[4] =0x04; reg[5] =0x05; reg[6] =0x06; reg[7] =0x07;
		mirroring =0;
		counter =0;
		latch =0;
		irq =false;
		EMU->SetIRQ(1);
	}
	sync();
	
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	
	// Mapper 48 denotes that the mirroring register is at E000.6 rather than 8000.6
	if (ROM->INES_MapperNum ==48) for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeMirroring);

	// If any game turns out to be bothered by the presence of the IRQ registers, a submapper to both mappers 33 and 48 would need to be allocated
	// to indicate TC0190
	readCHR =EMU->GetPPUReadHandler(0x4);
	for (int bank =0xC; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeIRQ);
	for (int bank =0x0; bank<=0x7; bank++) {
		EMU->SetPPUReadHandler(bank, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, readCHR);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, reg[i]);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BOOL(stateMode, offset, data, irq);
	if (ROM->INES_MapperNum ==48) SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum033 =33;
uint16_t mapperNum048 =48;
} // namespace

MapperInfo MapperInfo_033 ={
	&mapperNum033,
	_T("Taito TC0190/TC0390"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_048 ={
	&mapperNum048,
	_T("Taito TC0190+PAL16R4/TC0690"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
