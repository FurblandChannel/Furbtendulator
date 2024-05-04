#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prg     override &0x0F
#define	nrom    override &0x80
#define	nrom256 override &0x20

namespace {
uint8_t		override;
uint8_t		chr;
uint8_t		outer;

void	sync (void) {
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, nrom256? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, nrom256? prg | 1: prg);
	} else
		MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0xFF, chr <<8);	
	MMC3::syncMirror();
}

void	MAPINT	writeExtra (int bank, int addr, int val) {	
	if (addr &1)
		chr =val;
	else
		override =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		override =0x00;
		chr =0x00;
	}
	MMC3::reset(resetType);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeExtra);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, override);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =115;
uint16_t mapperNum2 =248;
} // namespace

MapperInfo MapperInfo_115 ={
	&mapperNum,
	_T("卡聖 SFC-02B/-03/-004"),
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
MapperInfo MapperInfo_248 ={
	&mapperNum2,
	_T("卡聖 SFC-02B/-03/-004"),
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