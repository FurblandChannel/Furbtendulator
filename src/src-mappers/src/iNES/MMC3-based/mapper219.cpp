#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
bool		extMode;

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(0x0F, reg <<4);
	MMC3::syncCHR(0x7F, reg <<7);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch(addr &1) {
		case 0: reg =reg &~1 | val >>3 &1; break;
		case 1: reg =reg &~2 | val >>4 &2; break;
	}
	sync();
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (~addr &1) {
		MMC3::writeReg(bank, addr, val);
		if (addr &2) extMode =!!(val &0x20);
	} else
	if (!extMode)
		MMC3::writeReg(bank, addr, val);
	else
	if (MMC3::pointer >=0x08 && MMC3::pointer <0x20) {
		int index =(MMC3::pointer -8) >>2;
		if (MMC3::pointer &1) {
			MMC3::reg[index] &=~0x0F;
			MMC3::reg[index] |=val >>1 &0x0F;
		} else {
			MMC3::reg[index] &=~0xF0;
			MMC3::reg[index] |=val <<4 &0xF0;
		}
		sync();
	} else
	if (MMC3::pointer >=0x25 && MMC3::pointer <=0x26) {
		MMC3::reg[6 | MMC3::pointer &1] =val >>5 &1 | val >>3 &2 | val >>1 &4 | val <<1 &8;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	reg =0xFF;
	extMode =false;
	MMC3::reset(resetType);
	
	EMU->SetCPUWriteHandler(0x5, writeReg);
	EMU->SetCPUWriteHandler(0x8, writeASIC);
	EMU->SetCPUWriteHandler(0x9, writeASIC);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	SAVELOAD_BOOL(stateMode, offset, data, extMode);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =219;
} // namespace

MapperInfo MapperInfo_219 = {
	&mapperNum,
	_T("卡聖 A9746"),
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
