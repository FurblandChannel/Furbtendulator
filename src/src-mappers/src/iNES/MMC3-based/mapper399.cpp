#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg[2];
uint8_t		chr[2];

void	sync (void) {
	if (ROM->INES2_SubMapper ==1) {
		EMU->SetPRG_ROM8(0x6, 0xFE);
		EMU->SetPRG_ROM8(0x8, prg[0] <<1 |0);
		EMU->SetPRG_ROM8(0xA, prg[0] <<1 |1);
	} else {
		MMC3::syncWRAM();
		EMU->SetPRG_ROM8(0x8, 0x00);
		EMU->SetPRG_ROM8(0xA, prg[0]);
	}
	
	EMU->SetPRG_ROM8(0xC, prg[1]);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	
	EMU->SetCHR_RAM4(0x0, chr[0]);
	EMU->SetCHR_RAM4(0x4, chr[1]);
	
	MMC3::syncMirror();	
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &1)
		prg[val >>7] =val;
	else
		chr[val >>7] =val;
	sync();
}

void	MAPINT	writeReg1 (int bank, int addr, int val) {
	switch(bank &~1) {
		case 0x8: MMC3::write(0xA, addr, val); break;
		case 0xA: MMC3::write(0xC, addr, val); break;
		case 0xC: MMC3::write(0xE, addr, val); break;
		case 0xE: writeReg(bank, addr, val); break;
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (int bank =0; bank <2; bank++) {
		prg[bank] =bank;
		chr[bank] =bank;
	}
	MMC3::reset(resetType);
	if (ROM->INES2_SubMapper ==1)
		for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg1);
	else
		for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =399;
} // namespace

MapperInfo MapperInfo_399 = {
	&mapperNum,
	_T("BATMAP-000"),
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