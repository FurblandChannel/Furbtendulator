#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define prg ((Latch::data &0x20? 0x01:0x00) | (Latch::data &0x80? 0x02: 0x00) | (Latch::data &0x08? 0x04: 0x00))
#define chr (Latch::data >>1 &3)
namespace {
uint8_t		prgSMB2J;
uint8_t		irq;
uint16_t	counter;
FCPUWrite	writeAPU;
	
void	sync (void) {
	EMU->SetPRG_ROM8 (0x6, 0x7);
	if (Latch::data &8) { // Normal NROM Mode
		if (Latch::data &0x80)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else
		if (Latch::data &0x40) {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		} else {
			EMU->SetPRG_ROM8 (0x8, prg <<1);
			EMU->SetPRG_ROM8 (0xA, prg <<1);
			EMU->SetPRG_ROM8 (0xC, prg <<1);
			EMU->SetPRG_ROM8 (0xE, prg <<1);			
		}
	} else { // SMB2J Mode
		EMU->SetPRG_ROM8(0x6, 0x7);
		EMU->SetPRG_ROM8(0x8, 0x0);
		EMU->SetPRG_ROM8(0xA, 0x1);
		EMU->SetPRG_ROM8(0xC, prgSMB2J &8 | prgSMB2J <<2 &4 | prgSMB2J >>1 &3);
		EMU->SetPRG_ROM8(0xE, 0x3);		
	}
	EMU->SetCHR_ROM8(0x0, chr);
	if (Latch::data &4)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (addr &0x020 && ~addr &0x040) {
		if (addr &0x100)
			irq =val;
		else {
			prgSMB2J =val;
			sync();
		}
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prgSMB2J =0;
	irq =0;
	counter =0;
	EMU->SetIRQ(1);
	Latch::reset(resetType);
	
	// The latch should not respond from $A000-$FFFF
	FCPUWrite writeNothing =EMU->GetCPUWriteHandler(0x6);
	for (int bank =0xA; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeNothing);
	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank<=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (irq &1)
		EMU->SetIRQ(++counter &0x1000? 0: 1);
	else {
		EMU->SetIRQ(1);
		counter =0;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prgSMB2J);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =416;
} // namespace

MapperInfo MapperInfo_416 = {
	&mapperNum,
	_T("N-32 4-in-1"),
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
