#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;
FCPUWrite writeAPU;

void	sync (void) {
	MMC3::syncPRG(0x3F, 0);
	EMU->SetPRG_RAM8(0x6, 0); // WRAM is always enabled

	if (reg &2)
		MMC3::syncCHR_RAM(0x03, 0x00);
	else
		MMC3::syncCHR_ROM(0xFF, 0x00);
	
	if (reg ==1) 	// VRAM
		EMU->SetCHR_RAM4(0x8, 1);
	else		// CIRAM
		MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		reg =val &3;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	reg =0;
	MMC3::reset(resetType);	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =512;
} // namespace

MapperInfo MapperInfo_512 ={
	&mapperNum,
	_T("中國大亨"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};