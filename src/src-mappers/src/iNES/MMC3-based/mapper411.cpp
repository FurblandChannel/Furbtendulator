#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define nrom    !!(reg[0] &0x40)
#define nrom256 !!(reg[0] &0x02)
#define prgOR     (reg[0] &0x01 | reg[0] >>2 &0x02 | reg[0] &0x04 | (reg[1] &0x08) | (reg[1] >>2 &0x10))
#define chrOR     (reg[1] <<5 &0x80 | reg[1] <<2 &0x100)
#define prgAND  (reg[1] &0x02? 0x1F: 0x0F)
#define chrAND  (reg[1] &0x02? 0xFF: 0x7F)

namespace {
uint8_t		reg[2];

void	sync (void) {
	if (nrom) {
		if (nrom256)
			EMU->SetPRG_ROM32(0x8, prgOR >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prgOR);
			EMU->SetPRG_ROM16(0xC, prgOR);
		}
	} else {
		MMC3::syncPRG(prgAND, (prgOR <<1) &~prgAND);
	}
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	write5 (int bank, int addr, int val) {
	if (addr &0x800) reg[addr &1] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg[0] =0x80;
	reg[1] =0x82; // Must boot up in 256 KiB mode
	
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, write5);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =411;
} // namespace

MapperInfo MapperInfo_411 ={
	&mapperNum,
	_T("A88S-1"),
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