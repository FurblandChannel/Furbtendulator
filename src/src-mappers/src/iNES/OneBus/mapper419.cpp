#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"
#include	"..\..\Hardware\Sound\Butterworth.h"
#include	"..\..\Hardware\Sound\s_ADPCM3Bit.h"

namespace {
ADPCM3Bit 	adpcm;
uint8_t         adpcmCount;
uint32_t        adpcmPeriod;
Butterworth	lowPassFilter(20, 1'789'772, 4'090'090 /512.0 /2.0);

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

static const uint8_t ppuMangle[16][6] = {
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 0: Normal
	{ 1, 0, 5, 4, 3, 2 }, 	// Submapper 1: Waixing VT03
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 2: Trump Grand
	{ 5, 4, 3, 2, 0, 1 }, 	// Submapper 3: Zechess
	{ 2, 5, 0, 4, 3, 1 }, 	// Submapper 4: Qishenglong
	{ 1, 0, 5, 4, 3, 2 }, 	// Submapper 5: Waixing VT02
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 6: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 7: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 8: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper 9: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper A: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper B: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper C: unused so far
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper D: Cube Tech (CPU opcode encryption only)
	{ 0, 1, 2, 3, 4, 5 }, 	// Submapper E: Karaoto (CPU opcode encryption only)
	{ 0, 1, 2, 3, 4, 5 }  	// Submapper F: Jungletac (CPU opcode encryption only)
};
static const uint8_t mmc3Mangle[16][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 0: Normal
	{ 5, 4, 3, 2, 1, 0, 6, 7 }, 	// Submapper 1: Waixing VT03
	{ 0, 1, 2, 3, 4, 5, 7, 6 }, 	// Submapper 2: Trump Grand
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 3: Zechess
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 4: Qishenglong
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 5: Waixing VT02
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 6: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 7: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 8: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper 9: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper A: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper B: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper C: unused so far
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper D: Cube Tech (CPU opcode encryption only)
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, 	// Submapper E: Karaoto (CPU opcode encryption only)
	{ 0, 1, 2, 3, 4, 5, 6, 7 }  	// Submapper F: Jungletac (CPU opcode encryption only)
};

void	MAPINT	writePPU (int bank, int addr, int val) {
	if (addr >=0x012 && addr <=0x017) addr =0x012 +ppuMangle[ROM->INES2_SubMapper][addr -0x012];
	OneBus::writePPU(bank, addr, val);
}

void	MAPINT	writeMMC3 (int bank, int addr, int val) {
	if (bank <=0x9 && ~addr &1) val =val &0xF8 | mmc3Mangle[ROM->INES2_SubMapper][val &0x07];
	OneBus::writeMMC3(bank, addr, val);
}

int	MAPINT	readADPCM (int bank, int addr) {
	int result =OneBus::readAPU(bank, addr);
	if (addr ==0x017) result =result &~0x18 | adpcm.getReady() *0x10 | adpcm.getClock() *0x08;
	return result;
}

void	checkADPCMPeriod () {
	if (adpcm.getPeriod() !=adpcmPeriod) {
		adpcmPeriod =adpcm.getPeriod();
		lowPassFilter.recalc(20, 1'789'772, 4'090'090 /adpcmPeriod /2.0);
	}
}

void	MAPINT	writeADPCM (int bank, int addr, int val) {
	switch(addr) {
		case 0x016:
			adpcm.setClock(!!(val &0x04));
			checkADPCMPeriod();

			OneBus::writeAPU(bank, addr, val);
			break;
		case 0x10F:			
			adpcm.setData(val &0x0F);
			break;
		default:
			OneBus::writeAPU(bank, addr, val);
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adpcmCount =0;
	adpcmPeriod =0;
	adpcm.reset();
	checkADPCMPeriod();
	
	OneBus::reset(resetType);
	
	EMU->SetCPUWriteHandler(0x2, writePPU);
	EMU->SetCPUReadHandler (0x4, readADPCM);
	EMU->SetCPUWriteHandler(0x4, writeADPCM);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeMMC3);
}

void	MAPINT	cpuCycle() {
	// Convert from 1.789773 MHz to 4.09 MHz clock (factor 16/7)
	adpcmCount +=16;
	while (adpcmCount >=7) {
		adpcmCount -=7;
		adpcm.run();
	}
	OneBus::cpuCycle();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =OneBus::saveLoad(stateMode, offset, data);
	offset =adpcm.saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, adpcmCount);
	if (stateMode ==STATE_LOAD) {
		sync();
		adpcmPeriod =0;
		checkADPCMPeriod();
	}
	return offset;
}

int	MAPINT	mapperSound (int cycles) {
	return lowPassFilter.output(adpcm.getOutput()*256 +1e-15);
}

uint16_t mapperNum =419;
} // namespace


MapperInfo MapperInfo_419 = {
	&mapperNum,
	_T("Taikee TK-8007 MCU"),
	COMPAT_FULL,
	load,
	reset,
	OneBus::unload,
	cpuCycle,
	OneBus::ppuCycle,
	saveLoad,
	mapperSound,
	NULL
};