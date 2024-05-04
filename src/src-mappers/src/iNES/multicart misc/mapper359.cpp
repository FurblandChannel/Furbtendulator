#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		prgAND;
uint8_t		chrAND;
uint16_t	prgOR;
uint16_t	chrOR;
uint8_t		mirroring;

uint16_n	counter;
uint8_t		pa12Filter;
bool		reload;
bool		irqEnabled;
bool		irqPA12;
bool		irqAutoEnable;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, prg[3] &prgAND |prgOR);
	EMU->SetPRG_ROM8(0x8, prg[0] &prgAND |prgOR);
	EMU->SetPRG_ROM8(0xA, prg[1] &prgAND |prgOR);
	EMU->SetPRG_ROM8(0xC, prg[2] &prgAND |prgOR);
	EMU->SetPRG_ROM8(0xE,   0xFF &prgAND |prgOR);

	if (ROM->CHRROMSize)
		if (ROM->INES_MapperNum ==540) {
			EMU->SetCHR_ROM2(0, chr[0]);
			EMU->SetCHR_ROM2(2, chr[1]);
			EMU->SetCHR_ROM2(4, chr[6]);
			EMU->SetCHR_ROM2(6, chr[7]);
		} else
			for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank] &chrAND |chrOR);
	else
		EMU->SetCHR_RAM8(0x0, 0);
	
	switch (mirroring) {
		case 0:	EMU->Mirror_V();  break;
		case 1:	EMU->Mirror_H();  break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[addr &3] =val;
	sync();
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	switch (addr &3) {
		case 0:	prgOR =(val &0x38) <<1;
			break;
		case 1:	switch (val &3) {
				case 0: prgAND =0x3F; break;
				case 1: prgAND =0x1F; break;
				case 2: prgAND =0x2F; break;
				case 3: prgAND =0x0F; break;
			}
			chrAND =val &0x40? 0xFF: 0x7F;
			break;
		case 2:	mirroring =val &3;
			break;
		case 3:	chrOR =val <<7;
			break;
	}
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &3 | (bank &1? 4: 0)] =val;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	switch (addr &3) {
		case 0: if (irqAutoEnable) irqEnabled =false;
			counter.b0 =val;
			break;
		case 1:	if (irqAutoEnable) irqEnabled =true;
			counter.b1 =val;
			reload =true;
			break;
		case 2: irqEnabled    =!!(val &0x1);
			irqPA12       =!!(val &0x2);
			irqAutoEnable =!!(val &0x4);
			break;
		case 3:	irqEnabled =!!(val &1);
			break;
	}
	EMU->SetIRQ(1);
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType == RESET_HARD) {
		prgAND =0x3F;
		prgOR  =0x00;
		chrAND =0xFF;
		chrOR  =0x00;
		for (int i =0; i <4; i++) prg[i] =0xFC |i;
		for (int i =0; i <8; i++) chr[i] =i;
		mirroring =0;

		counter.s0 =0;
		pa12Filter =0;
		reload =false;
		irqEnabled =false;
		irqPA12 =false;
		irqAutoEnable =false;
	}
	sync();
	FDSsound::ResetBootleg(resetType);
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeMode);
	EMU->SetCPUWriteHandler(0xA, writeCHR);
	EMU->SetCPUWriteHandler(0xB, writeCHR);
	EMU->SetCPUWriteHandler(0xC, writeIRQ);
}

void	MAPINT	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
	if (irqEnabled && !irqPA12 && counter.s0 && !--counter.s0) EMU->SetIRQ(0);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter && irqPA12) {
			if (!counter.b0 || reload)
				counter.b0 =counter.b1;
			else
				counter.b0--;
			if (!counter.b0 && irqEnabled) EMU->SetIRQ(0);
			reload =false;
		}
		pa12Filter =5;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(stateMode, offset, data, prg[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, chr[i]);
	SAVELOAD_BYTE(stateMode, offset, data, prgAND);
	SAVELOAD_WORD(stateMode, offset, data, prgOR);
	SAVELOAD_BYTE(stateMode, offset, data, chrAND);
	SAVELOAD_WORD(stateMode, offset, data, chrOR);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_WORD(stateMode, offset, data, counter.s0);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BOOL(stateMode, offset, data, reload);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	SAVELOAD_BOOL(stateMode, offset, data, irqPA12);
	SAVELOAD_BOOL(stateMode, offset, data, irqAutoEnable);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum359 =359;
uint16_t MapperNum540 =540;
} // namespace

MapperInfo MapperInfo_359 = {
	&MapperNum359,
	_T("GCL8050/SB-5013/841242C"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};

MapperInfo MapperInfo_540 = {
	&MapperNum540,
	_T("82112C"),
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