#include	"..\DLL\d_iNES.h"
#include	<queue>
#include	"..\Hardware\simplefdc.hpp"
namespace {
FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPUWrite	writeAPU;
FDC		fdc;

uint8_t		ram[2048];

uint8_t		prgBank;
uint8_t		biosBank;

bool		enableIRQ;
uint32_t	mask;
uint32_t	counter;

void	sync (void) {
	for (int bank =0x6; bank<=0xD; bank++) EMU->SetPRG_OB4(bank);
	EMU->SetPRG_ROM8(0xE, biosBank);
	EMU->SetCHR_RAM8(0x0, 0);
	EMU->Mirror_V();
}

int	MAPINT	readReg (int bank, int addr) {
	if (addr &0x800) {
		return ram[addr &0x7FF];
	} else
	switch (addr) {
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			return fdc.readIO(addr);
		case 0x04C:
			return fdc.irqRaised()? 0x80: 0x00;
		default:
			return readAPU(bank, addr);
	}
	
}
int	MAPINT	readRegDebug (int bank, int addr) {
	if (addr &0x800) {
		return ram[addr &0x7FF];
	} else
	switch (addr) {
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			return fdc.readIODebug(addr);
		case 0x04C:
			return fdc.irqRaised()? 0x80: 0x00;
		default:
			return readAPUDebug(bank, addr);
	}
	
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &0x800) {
		ram[addr &0x7FF] =val;
	} else
	switch (addr) {
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			fdc.writeIO(addr, val);
			break;		
		case 0x048: // value ignored
			enableIRQ =true;
			counter =0;
			break;
		case 0x049: // value ignored. What is the difference to 404B?
			enableIRQ =false;
			EMU->SetIRQ(1);
			break;
		case 0x04A:
		case 0x04D:
		case 0x04E:
		case 0x04F:
			EMU->DbgOut(L"Unknown 4%03X", addr);
			break;
		case 0x04B: // value ignored!
			enableIRQ =false;
			EMU->SetIRQ(1);
			break;
		case 0x04C:
			EMU->SetIRQ(1);
			mask =0;
			if (val &0x08) {
				uint8_t whichBit =val &7;
				mask |=(1 <<whichBit) <<7;
				if (whichBit ==0) EMU->SetIRQ(0);
			}
			mask |= 1 <<23;
			break;
		default:
			writeAPU(bank, addr, val);
			break;
	}
}
void	MAPINT	writeBank (int bank, int addr, int val) {
	switch (addr &1) {
		case 0:	prgBank =val &0x7F; break;
		case 1:	biosBank =val &0x01; break;
	}
	sync();
}

BOOL	MAPINT	load (void) {
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prgBank =0;
	biosBank =0;
	counter =0;
	sync();
	
	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	writeAPU     =EMU->GetCPUWriteHandler    (0x4);
	EMU->SetCPUReadHandler     (0x4, readReg     );
	EMU->SetCPUReadHandlerDebug(0x4, readRegDebug);
	EMU->SetCPUWriteHandler    (0x4, writeReg    );	
	EMU->SetCPUWriteHandler    (0xE, writeBank   );
}

void	MAPINT	cpuCycle (void) {
	if (ROM->changedDisk35) {
		fdc.ejectDisk(0);
		if (ROM->diskData35 !=NULL) fdc.insertDisk(0, &(*ROM->diskData35)[0], ROM->diskData35->size(), false);
	}
	fdc.run();
	if (!(counter &mask) && ++counter &mask && enableIRQ) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: ram) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BYTE(stateMode, offset, data, prgBank);
	SAVELOAD_BYTE(stateMode, offset, data, biosBank);
	
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_LONG(stateMode, offset, data, mask);
	SAVELOAD_LONG(stateMode, offset, data, counter);
	
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =761; /* PRELIMINARY mapper number */
} // namespace

MapperInfo MapperInfo_761 ={
	&MapperNum,
	_T("Bung MFC"),
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