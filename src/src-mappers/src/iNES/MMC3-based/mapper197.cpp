#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		outerBank;

void	sync0 (void) {
	MMC3::syncPRG(0x3F, 0x00);
	EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0));
	EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(1));
	EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4));
	EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(5));
	MMC3::syncMirror();
}

void	sync1 (void) {
	MMC3::syncPRG(0x3F, 0x00);
	EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(2));
	EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3));
	EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(6));
	EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7));
	MMC3::syncMirror();
}

void	sync2 (void) {
	MMC3::syncPRG(0x3F, 0x00);
	EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0));
	EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3));
	EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4));
	EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7));
	MMC3::syncMirror();
}

void	sync3 (void) {
	MMC3::syncPRG(outerBank &0x08? 0x0F: 0x1F, outerBank <<4);
	EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) | outerBank <<7 &0x100);
	EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(1) | outerBank <<7 &0x100);
	EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) | outerBank <<7 &0x100);
	EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(5) | outerBank <<7 &0x100);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	switch(ROM->INES2_SubMapper) {
		case 1:	MMC3::load(sync1); break;
		case 2:	MMC3::load(sync2); break;
		case 3:	MMC3::load(sync3); break;
		default:MMC3::load(sync0); break;
	}
	return TRUE;
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank =val;
	sync3();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	MMC3::reset(resetType);	
	if (ROM->INES2_SubMapper ==3) MMC3::setWRAMCallback(NULL, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (ROM->INES2_SubMapper ==3) SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =197;
} // namespace

MapperInfo MapperInfo_197 ={
	&mapperNum,
	_T("TLROM-512"),
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