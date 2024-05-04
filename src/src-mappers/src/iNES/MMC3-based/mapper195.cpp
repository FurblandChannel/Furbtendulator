#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
FPPUWrite	writeCHR;
uint8_t		maskCHRBank;
uint8_t		maskCompare;

void	sync (void) {
	if (ROM->PRGRAMSize >8192) EMU->SetPRG_RAM4(0x5, 2);
	MMC3::syncWRAM();
	MMC3::syncPRG(0x3F, 0);
	for (int bank =0; bank <8; bank++) {
		int val =MMC3::getCHRBank(bank);
		if ((val &maskCHRBank) ==maskCompare)
			EMU->SetCHR_RAM1(bank, val);
		else
			EMU->SetCHR_ROM1(bank, val);
	}
	MMC3::syncMirror();
}

static const uint8_t compareMasks[8] = {
	0x28, 0x00, 0x4C, 0x64, 0x46, 0x7C, 0x04, 0xFF
};
void	MAPINT	interceptCHRWrite (int bank, int addr, int val) {
	uint8_t chr =MMC3::getCHRBank(bank);
	if (chr &0x80) {
		//EMU->DbgOut(L"CHR %02X", chr);
		if (chr &0x10) {
			maskCHRBank =0x00;
			maskCompare =0xFF; // OFF
		} else {
			uint8_t index =(chr &0x02? 1: 0) | (chr &0x08? 2: 0) | (chr &0x40? 4: 0);
			maskCHRBank =chr &0x40? 0xFE: 0xFC;
			maskCompare =compareMasks[index];
		}
		sync();
	}
	writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		maskCHRBank =0xFC;
		maskCompare =0x00;
	}
	MMC3::reset(resetType);
	writeCHR =EMU->GetPPUWriteHandler(0x0);
	for (int bank =0x0; bank<=0x7; bank++) EMU->SetPPUWriteHandler(bank, interceptCHRWrite);
}


int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, maskCHRBank);
	SAVELOAD_BYTE(stateMode, offset, data, maskCompare);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =195;
} // namespace

MapperInfo MapperInfo_195 ={
	&mapperNum,
	_T("外星 FS303"),
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