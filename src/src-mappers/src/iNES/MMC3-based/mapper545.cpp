#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t 	reg;

void	sync (void) {
	uint8_t prgOR  =( reg &3) <<4;
	if (reg &8) {	// 256 KiB Split-PRG mode
		for (int bank =0; bank <4; bank++) {
			int val =MMC3::getPRGBank(bank) &0x1F;
			val =val &0x0F | (val &0x10? prgOR: 0x40);
			EMU->SetPRG_ROM8(0x8 +bank*2, val);
		}
	} else
		MMC3::syncPRG(0x0F, prgOR);		
	MMC3::syncWRAM();

	int chrOR  =( reg &3) <<7;
	int chrA16 =(~reg &4) <<4;
	MMC3::syncCHR_ROM(0x7F, chrOR | chrA16);

	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &0x020 && addr &0x100) {
		reg =val;
		sync();
	} else
		MMC3::writeIRQEnable(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	reg =0x00;
	MMC3::reset(resetType);	
	EMU->SetCPUWriteHandler(0xF, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =545;
} // namespace

MapperInfo MapperInfo_545 ={
	&mapperNum,
	_T("ST-80"),
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