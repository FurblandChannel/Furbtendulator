#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg[4];
uint8_t		keyboardRow;
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	MMC3::syncMirror();
	MMC3::syncWRAM();
	MMC3::syncPRG_4(0x1F, 0x1F, 0x1F, 0x1F, prg[0], prg[1], prg[2], prg[3]);
	MMC3::syncCHR(0xFF, 0);
}

int	MAPINT	readReg (int bank, int addr) {
	int keys =0;
	switch (addr) {
		case 0x906: // Read keyboard. Translate to Subor Keyboard reads.
			writeAPU(0x4, 0x016, 0x04 | 0x01); // Reset keyboard
			for (int row =0; row <=keyboardRow; row++) {
				writeAPU(0x4, 0x016, 0x04 | 0x00);
				keys =readAPU(0x4, 0x17) >>1 &0x0F;
				
				writeAPU(0x4, 0x016, 0x04 | 0x02);				
				keys |= readAPU(0x4, 0x17) <<3 &0xF0;
			}
			return keys;
		case 0xC03:
			return 0x00; //$80: PCM Chip Busy
			break;
		default:
			return readAPU(bank, addr);
			break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	switch (addr) {
		case 0x904:
			keyboardRow =val;
			break;
		case 0x905:
			// Keyboard, written to at end of VBlank handler
			break;
		case 0xC00: case 0xC01:
			prg[addr &3] =val &0xE0;
			sync();
			break;
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: prg) c =0xE0;
	MMC3::reset(resetType);
	
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, keyboardRow);
	if (stateMode== STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =531;
} // namespace

MapperInfo MapperInfo_531 ={
	&mapperNum,
	_T("LittleCom PC-95"),
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
