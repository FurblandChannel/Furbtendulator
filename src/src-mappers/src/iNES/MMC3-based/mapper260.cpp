#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define mode   (reg[0] &0x7F)
#define locked (reg[0] &0x80)
#define prg     reg[1]
#define chr     reg[2]
#define latch   reg[3]

namespace {
uint8_t		reg[4];

void	sync (void) {
	switch(mode) {
		case 0: MMC3::syncPRG    (0x1F, prg <<1 &~0x1F); // MMC3, 256 KiB PRG, 256 KiB CHR
			MMC3::syncCHR_ROM(0xFF, chr <<3 &~0xFF);
			break;
		case 1: MMC3::syncPRG    (0x1F, prg <<1 &~0x1F); // MMC3, 256 KiB PRG, 128 KiB CHR
			MMC3::syncCHR_ROM(0x7F, chr <<3 &~0x7F);
			break;
		case 2: MMC3::syncPRG    (0x0F, prg <<1 &~0x0F); // MMC3, 128 KiB PRG, 256 KiB CHR
			MMC3::syncCHR_ROM(0xFF, chr <<3 &~0xFF);
			break;
		case 3: MMC3::syncPRG    (0x0F, prg <<1 &~0x0F); // MMC3, 128 KiB PRG, 128 KiB CHR
			MMC3::syncCHR_ROM(0x7F, chr <<3 &~0x7F);
			break;
		case 4: EMU->SetPRG_ROM16(0x8, prg); // NROM-128
			EMU->SetPRG_ROM16(0xC, prg);
			EMU->SetCHR_ROM8(0, chr);
			break;
		case 5: EMU->SetPRG_ROM32(0x8, prg >>1); // NROM-256
			EMU->SetCHR_ROM8(0, chr);
			break;
		case 6: EMU->SetPRG_ROM32(0x8, prg >>1); // CNROM, 16 KiB CHR
			EMU->SetCHR_ROM8(0, chr &~1 | latch &1);
			break;
		case 7: EMU->SetPRG_ROM32(0x8, prg >>1); // CNROM, 32 KiB CHR
			EMU->SetCHR_ROM8(0, chr &~3 | latch &3);
			break;
	}
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue &3 | *EMU->OpenBus &~3;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
		reg[addr &3] =val;
		sync();
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (mode >=6) {
		latch =val;
		sync();
	} else
		MMC3::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
	
	EMU->SetCPUReadHandler(0x5, readDIP);
	EMU->SetCPUWriteHandler(0x5, writeReg);	
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =260;
} // namespace

MapperInfo MapperInfo_260 ={
	&mapperNum,
	_T("HP10xx-HP20xx"),
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