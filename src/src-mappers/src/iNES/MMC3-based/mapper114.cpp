#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[4];

void	sync (void) {
	if (reg[0] &0x80) {
		uint8_t prg =reg[0] &0x0F;
		EMU->SetPRG_ROM16(0x8, reg[0] &0x20? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, reg[0] &0x20? prg | 1: prg);
	} else
		MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0xFF, 0x00);
	MMC3::syncMirror();
}
void	sync182 (void) {
	int prgAND =reg[1] &0x20? 0x1F: 0x0F;
	int chrAND =reg[1] &0x40? 0xFF: 0x7F;
	int prgOR =(reg[1] &0x02? 0x10: 0x00) | (reg[1] &0x10? 0x020: 0x00);
	int chrOR =(reg[1] &0x02? 0x80: 0x00) | (reg[1] &0x10? 0x100: 0x00);		
	if (reg[0] &0x80) {
		uint8_t prg =reg[0] &0x0F;
		EMU->SetPRG_ROM16(0x8, reg[0] &0x20? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, reg[0] &0x20? prg | 1: prg);
	} else
		MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR_ROM(chrAND, chrOR);
	MMC3::syncMirror();
}

int	MAPINT	readDIP (int bank, int addr) {
	if ((addr &3) ==2)
		return ROM->dipValue &7 | *EMU->OpenBus &~7;
	else
		return *EMU->OpenBus;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg[1] &1) reg[addr &3] =val;
	MMC3::sync();
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	static const uint16_t addressLUT[4][8] ={
		{ 0xA001, 0xA000, 0x8000, 0xC000, 0x8001, 0xC001, 0xE000, 0xE001 }, // 0 The Lion King, Aladdin
		{ 0xA001, 0x8001, 0x8000, 0xC001, 0xA000, 0xC000, 0xE000, 0xE001 }, // 1 Boogerman
		{ 0xC001, 0x8000, 0x8001, 0xA000, 0xA001, 0xE001, 0xE000, 0xC000 }, // 2 2-in-1 The Lion King/Bomber Boy
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }  // 3 -
	};
	static const uint8_t dataLUT[4][8] ={
		{ 0, 3, 1, 5, 6, 7, 2, 4 }, // 0 The Lion King, Aladdin
		{ 0, 2, 5, 3, 6, 1, 7, 4 }, // 1 Boogerman
		{ 0, 6, 3, 7, 5, 2, 4, 1 }, // 2 2-in-1 The Lion King/Bomber Boy
		{ 0, 1, 2, 3, 4, 5, 6, 7 }  // 3 -
	};

	int lutValue =addressLUT[ROM->INES2_SubMapper &3][bank &6 | addr &1];
	if (lutValue ==0x8000) val =val &0xC0 | dataLUT[ROM->INES2_SubMapper &3][val &7];
	MMC3::write(lutValue >>12, lutValue &1, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(ROM->INES_MapperNum ==182? sync182: sync, MMC3Type::NEC);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
	for (int i =0x6; i<=0x7; i++) {
		EMU->SetCPUReadHandler(i, readDIP);
		EMU->SetCPUReadHandlerDebug(i, readDIP);
		EMU->SetCPUWriteHandler(i, writeReg);
	}
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeASIC);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum114 =114;
uint16_t mapperNum182 =182;
} // namespace

MapperInfo MapperInfo_114 ={
	&mapperNum114,
	_T("6122"),
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
MapperInfo MapperInfo_182 ={
	&mapperNum182,
	_T("YH-001"),
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