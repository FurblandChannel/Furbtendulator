#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_FME7.h"

#define	IRQ_ACKNOWLEDGE	0x01
#define	IRQ_ENABLE	0x02
#define	IRQ_CYCLE_MODE	0x04

namespace {
uint8_t		OuterBank, IRQControl, IRQCounter, IRQLatch;
int16_t		IRQCycles;

void	Sync (void) {
	int PRGMask =0x0F | OuterBank;
	if (FME7::prg[0] ==0x01)
		EMU->SetPRG_RAM8(0x6, 0);
	else
		EMU->SetPRG_ROM8(0x6, (FME7::prg[0] &PRGMask) +OuterBank);
	EMU->SetPRG_ROM8(0x8, (FME7::prg[1] &PRGMask) +OuterBank);
	EMU->SetPRG_ROM8(0xA, (FME7::prg[2] &PRGMask) +OuterBank);
	EMU->SetPRG_ROM8(0xC, (        0xFE &PRGMask) +OuterBank);
	EMU->SetPRG_ROM8(0xE, (        0xFF &PRGMask) +OuterBank);
	FME7::syncCHR(0xFF, OuterBank <<4);
	FME7::syncMirror();
}

void	MAPINT	WriteAC (int Bank, int Addr, int Val) {
		Addr &=0xF;
		switch(Addr) {
			case 0xD:	IRQControl =Val;
					if (IRQControl &IRQ_ENABLE) {
						IRQCounter =IRQLatch;
						IRQCycles =341;
					}
					EMU->SetIRQ(1);
					break;
			case 0xE:	if (IRQControl &IRQ_ACKNOWLEDGE)
						IRQControl |= IRQ_ENABLE;
					else
						IRQControl &=~IRQ_ENABLE;
					EMU->SetIRQ(1);			
					break;
			case 0xF:	IRQLatch =Val;
					break;
			default:	FME7::writeIndex(0x8, 0, Addr);
					FME7::writeData (0xA, 0, Val);
					break;
		}
		OuterBank =(Bank ==0xC)? 0x10: 0x00;
		Sync();
}

BOOL	MAPINT	Load (void) {
	FME7::load(Sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	IRQControl =IRQCounter =IRQLatch =IRQCycles =0;
	FME7::reset(ResetType);
	EMU->SetCPUWriteHandler(0xA, WriteAC);
	EMU->SetCPUWriteHandler(0xC, WriteAC);	
	Sync();
}

void	MAPINT	Unload (void) {
	FME7::unload();
}

void	MAPINT	CPUCycle (void) {
	if ((IRQControl &IRQ_ENABLE) && ((IRQControl &IRQ_CYCLE_MODE) || ((IRQCycles -=3) <= 0))) {
		if (~IRQControl &IRQ_CYCLE_MODE) IRQCycles +=341;
		if (IRQCounter ==0xFF) {
			IRQCounter = IRQLatch;
			EMU->SetIRQ(0);
		}
		else
			IRQCounter++;
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = FME7::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	SAVELOAD_BYTE(mode, offset, data, IRQControl);
	SAVELOAD_BYTE(mode, offset, data, IRQCounter);
	SAVELOAD_BYTE(mode, offset, data, IRQLatch);
	SAVELOAD_WORD(mode, offset, data, IRQCycles);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =528;
} // namespace

MapperInfo MapperInfo_528 ={
	&MapperNum,
	_T("831128C"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};