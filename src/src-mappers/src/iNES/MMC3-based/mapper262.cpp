#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
union {
	struct {
		unsigned ppu1000: 1;
		unsigned ppu1800: 1;
		unsigned ppu0800: 1;
		unsigned ppu0000: 1;
		unsigned: 2;
		unsigned chrRAM: 1;
	};
	uint8_t data;
} mode;
FCPURead readAPU;
FCPUWrite writeAPU;

void	sync (void) {
	MMC3::syncPRG(0x3F, 0);
	if (mode.chrRAM)
		EMU->SetCHR_RAM8(0, 0);
	else
		MMC3::syncCHR_ROM(0xFF, mode.ppu0000? 0x100: 0x000,
	                                mode.ppu0800? 0x100: 0x000, 
	                                mode.ppu1000? 0x100: 0x000, 
	                                mode.ppu1800? 0x100: 0x000);
	EMU->Mirror_4();
}

int	MAPINT	readDIP (int bank, int addr) {
	if (addr &0x100)
		return ROM->dipValue;
	else
		return readAPU(bank, addr);
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		mode.data =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) mode.data =0;
	MMC3::reset(resetType);
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readDIP);
	EMU->SetCPUWriteHandler(0x4, writeMode);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mode.data);
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =262;
} // namespace

MapperInfo MapperInfo_262 ={
	&mapperNum,
	_T("Street Heroes"),
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