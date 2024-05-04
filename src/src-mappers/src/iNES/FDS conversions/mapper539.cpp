#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t		prg;
bool		horizontalMirroring;
FCPURead	cpuRead;
FCPUWrite	cpuWrite;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, 0xD);
	EMU->SetPRG_ROM8(0x8, 0xC);
	EMU->SetPRG_ROM8(0xA, prg);
	EMU->SetPRG_ROM8(0xC, 0xE);
	EMU->SetPRG_ROM8(0xE, 0xF);
	EMU->SetCHR_RAM8(0x0, 0x0);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

/*
$C000-$D1FF	RAM	0000-11FF	Addr &0x1FFF
$8200-$82FF	RAM	1200-12FF	Addr &0x1FFF | 0x1000
$6000-$60FF	RAM	1800-18FF	Addr &0x1FFF | 0x1800
$6200-$62FF	RAM	1A00-1AFF	Addr &0x1FFF | 0x1800
$6400-$65FF	RAM	1C00-1DFF	Addr &0x1FFF | 0x1800
$DF00-$DFFF	RAM	1F00-1FFF	Addr &0x1FFF


*/

int	MAPINT	interceptCPURead (int bank, int addr) {
	int fullAddr =(bank <<12) | addr;
	int wramAddr =(fullAddr &0x1FFF) | (bank <0xC? 0x1000: 0x0000) | (bank <0x8? 0x0800: 0x000);
	switch(fullAddr >>8) {
		case 0x60: case 0x62: case 0x64: case 0x65: case 0x82:
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1: case 0xDF: 
			return ROM->PRGRAMData[wramAddr];
		default:
			return cpuRead(bank, addr);
	}
}
void	MAPINT	interceptCPUWrite (int bank, int addr, int val) {
	int fullAddr =(bank <<12) | addr;
	int wramAddr =(fullAddr &0x1FFF) | (bank <0xC? 0x1000: 0x0000) | (bank <0x8? 0x0800: 0x000);
	switch(fullAddr >>8) {
		case 0x60: case 0x62: case 0x64: case 0x65: case 0x82:
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1: case 0xDF: 
			ROM->PRGRAMData[wramAddr] =val;
			break;
		default:
			cpuWrite(bank, addr, val);
			break;
	}
}
void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
	interceptCPUWrite(bank, addr, val);
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	if (bank !=0x4 || addr ==0x25) {
		horizontalMirroring =!!(val &8);
		sync();
	}
	if (bank ==0x4)
		FDSsound::Write4(bank, addr, val);
	else
		interceptCPUWrite(bank, addr, val);
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		horizontalMirroring =true;
		// Uninitialized CHR-RAM --- initialize it to match real hardware screenshot
		for (unsigned i =0; i <ROM->CHRRAMSize; i++) ROM->CHRRAMData[i] =i&2? 0xFF: 0x00;
	}
	sync();
	FDSsound::ResetBootleg(resetType);

	cpuRead =EMU->GetCPUReadHandler(0x8);
	cpuWrite =EMU->GetCPUWriteHandler(0x8);
	for (int i =0x6; i<=0xF; i++) {
		EMU->SetCPUReadHandler(i, interceptCPURead);
		EMU->SetCPUReadHandlerDebug(i, interceptCPURead);
		EMU->SetCPUWriteHandler(i, interceptCPUWrite);
	}
	EMU->SetCPUWriteHandler(0xA, writePRG);
	EMU->SetCPUWriteHandler(0x4, writeMirroring);
	EMU->SetCPUWriteHandler(0xF, writeMirroring);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =539;
} // namespace

MapperInfo MapperInfo_539 ={
	&mapperNum,
	_T("Parthenaの鏡 (bootleg)"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};
