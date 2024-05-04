#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
bool		nrom;
bool		nrom256;

uint8_t		prg;
uint8_t		prgOR;
uint8_t		prgOR128;
uint8_t		prgAND;

uint16_t	chrOR;
uint8_t		chrOR128;
uint16_t	chrAND;

uint8_t		array;
uint8_t		pattern;

void	sync (void) {
	if (ROM->dipValueSet) {
		if (ROM->dipValue &1) {
			prgOR &=ROM->dipValue;
			chrOR &=ROM->dipValue <<3 |7;
		} else
			prgOR |=ROM->dipValue;
	}
	if (nrom) {
		uint8_t bank =prg &(prgAND >>1) | ((prgOR &~prgAND) >>1) | ((prgOR128 &~prgAND) >>1);
		EMU->SetPRG_ROM16(0x8, nrom256? bank &~1: bank);
		EMU->SetPRG_ROM16(0xC, nrom256? bank | 1: bank);
	} else
		MMC3::syncPRG(prgAND, prgOR &~prgAND | prgOR128 &~prgAND);
	MMC3::syncWRAM();
	MMC3::syncCHR_ROM(chrAND, chrOR &~chrAND | chrOR128 &~chrAND);
	MMC3::syncMirror();
}

int	MAPINT	readArray (int bank, int addr) {
		//   0     1     2     3     4     5     6     7
	static const uint8_t arrayLUT[8][8] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0 Super Hang-On
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00 }, // 1 Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00 }, // 2 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00 }, // 3 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 6
		{ 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00 }  // 7 (default) Blood of Jurassic
	};
	return arrayLUT[array][addr &7] &0x0F | *EMU->OpenBus &0xF0;
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	switch(addr &7) {
		case 0:	prg =val &0x0F;
			nrom256 =!!(val &0x20);
			prgAND =val &0x40? 0x0F: 0x1F;
			chrAND =val &0x40? 0x7F: 0xFF;
			nrom =!!(val &0x80);
			sync();
			break;
		case 1:	if (ROM->INES2_SubMapper ==1) {
				prgOR = val <<5 &0x60 | val <<4 &0x80;
				chrOR = val <<7 &0x700;
			} else {
				prgOR = val <<5 &0x60;
				chrOR = val <<6 &0x300;
			}
			prgOR128 =val     &0x10;
			chrOR128 =val <<2 &0x80;
			sync();
			break;
		case 2:	array =val &7;
			break;
		case 7:	pattern =val &7;
			break;
	}
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	static const uint16_t addressLUT[8][8] ={
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 0
		{ 0xA001, 0xA000, 0x8000, 0xC000, 0x8001, 0xC001, 0xE000, 0xE001 }, // 1
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 2
		{ 0xC001, 0x8000, 0x8001, 0xA000, 0xA001, 0xE001, 0xE000, 0xC000 }, // 3
		{ 0xA001, 0x8001, 0x8000, 0xC000, 0xA000, 0xC001, 0xE000, 0xE001 }, // 4
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 5
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 6
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }  // 7
	};
	static const uint8_t dataLUT[8][8] ={
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 0
		{ 0, 2, 6, 1, 7, 3, 4, 5 }, // 1
		{ 0, 5, 4, 1, 7, 2, 6, 3 }, // 2
		{ 0, 6, 3, 7, 5, 2, 4, 1 }, // 3
		{ 0, 2, 5, 3, 6, 1, 7, 4 }, // 4
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 5
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 6
		{ 0, 1, 2, 3, 4, 5, 6, 7 }  // 7
	};
	int lutValue =addressLUT[pattern][bank &6 | addr &1];
	if (lutValue ==0x8000) val =val &0xC0 | dataLUT[pattern][val &7];
	MMC3::write(lutValue >>12, lutValue &1, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::NEC);
	if (ROM->INES_Version <2 && ROM->PRGROMSize >=2048*1024) ROM->INES2_SubMapper =1;
	MapperInfo_215.Description =(ROM->INES2_SubMapper ==1)? _T("Realtec 8237A"): _T("Realtec 823x");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	nrom =false;
	nrom256 =false;

	prg =0x00;
	prgOR =0xE0;
	prgOR128 =0x00;
	prgAND =0x1F;

	chrOR =0x700;
	chrOR128 =0x00;
	chrAND =0xFF;

	array =7; // for "Blood of Jurassic"
	pattern =4; // For single-cartridge "Boogerman" (should be Mapper 114.1)
	MMC3::reset(RESET_HARD);
	EMU->SetCPUReadHandler(0x5, readArray);
	EMU->SetCPUReadHandlerDebug(0x5, readArray);
	EMU->SetCPUWriteHandler(0x5, writeExtra);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeASIC);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, nrom);
	SAVELOAD_BOOL(stateMode, offset, data, nrom256);

	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, prgOR);
	SAVELOAD_BYTE(stateMode, offset, data, prgOR128);
	SAVELOAD_BYTE(stateMode, offset, data, prgAND);

	SAVELOAD_WORD(stateMode, offset, data, chrOR);
	SAVELOAD_BYTE(stateMode, offset, data, chrOR128);
	SAVELOAD_BYTE(stateMode, offset, data, chrAND);

	SAVELOAD_BYTE(stateMode, offset, data, array);
	SAVELOAD_BYTE(stateMode, offset, data, pattern);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum114 =215;
uint16_t mapperNum182 =258;
} // namespace

MapperInfo MapperInfo_215 ={
	&mapperNum114,
	_T("Realtec 823x(A)"),
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
MapperInfo MapperInfo_258 ={
	&mapperNum182,
	_T("Shanghai Paradise 158B"),
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