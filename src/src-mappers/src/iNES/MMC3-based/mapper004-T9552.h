#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace T9552 {
uint8_t		pattern;

static const uint8_t prgPattern[4][4] = {
	{ 16 -13, 17 -13, 15 -13, 14 -13 },
	{ 17 -13, 16 -13, 14 -13, 15 -13 },
	{ 14 -13, 15 -13, 16 -13, 17 -13 },
	{ 15 -13, 14 -13, 17 -13, 16 -13 }
};
static const uint8_t chrPattern[8][6] = {
	{ 15 -10, 12 -10, 16 -10, 17 -10, 14 -10, 13 -10 },
	{ 14 -10, 15 -10, 13 -10, 12 -10, 17 -10, 16 -10 },
	{ 12 -10, 13 -10, 14 -10, 15 -10, 16 -10, 17 -10 },
	{ 16 -10, 14 -10, 12 -10, 13 -10, 17 -10, 15 -10 },
	{ 15 -10, 13 -10, 17 -10, 16 -10, 12 -10, 14 -10 },
	{ 14 -10, 12 -10, 15 -10, 16 -10, 17 -10, 13 -10 },
	{ 13 -10, 16 -10, 14 -10, 15 -10, 12 -10, 17 -10 },
	{ 12 -10, 15 -10, 16 -10, 17 -10, 13 -10, 14 -10 }
};

int	scrambleBankOrder (int val, const uint8_t* sourceBits, const uint8_t* targetBits, int patternLength) {
	int result =0;
	for (int bit =0; bit <8; bit++) if (val &(1 <<bit)) {
		int index;
		for (index =0; index <patternLength; index++) if (sourceBits[index] ==bit) break;
		result |= 1 <<(index ==patternLength? bit: targetBits[index]);
	}
	return result;
}

void	sync (void) {
	MMC3::syncWRAM();	
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(bank *2 +0x8, scrambleBankOrder(MMC3::getPRGBank(bank), prgPattern[pattern &3], prgPattern[ROM->INES_MapperNum ==249? 0: 2], 4));
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank,         scrambleBankOrder(MMC3::getCHRBank(bank), chrPattern[pattern &7], chrPattern[ROM->INES_MapperNum ==249? 0: 2], 6));
	MMC3::syncMirror();
}

void	MAPINT	writePattern (int bank, int addr, int val) {
	pattern =val;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::Sharp);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	pattern =0;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writePattern);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, pattern);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =4;
MapperInfo MapperInfo_T9552 ={
	&mapperNum,
	_T("43-319"),
	COMPAT_FULL,
	load,
	reset,
	::unload,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
} // namespace
