#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
int	regNum;
int	prgAND, prgOR;
int	chrAND, chrOR;
bool	vrc4mode;
bool	locked;

void	sync (void) {
	if (vrc4mode) {
		VRC24::syncPRG(prgAND, prgOR);
		VRC24::syncCHR(chrAND, chrOR);
		VRC24::syncMirror();
	} else {
		MMC3::syncWRAM();
		MMC3::syncPRG(prgAND, prgOR);
		MMC3::syncCHR_ROM(chrAND, chrOR);
		MMC3::syncMirror();
	}

	*EMU->multiPRGStart  =ROM->PRGROMData +(prgOR <<13);
	*EMU->multiCHRStart  =ROM->CHRROMData +(chrOR <<10);
	*EMU->multiPRGSize   =(prgAND +1) <<13;
	*EMU->multiCHRSize   =(chrAND +1) <<10;
	*EMU->multiMapper    =(*EMU->multiPRGSize <=32768 && *EMU->multiCHRSize <=8192)? 0: 4;
	*EMU->multiMirroring =MMC3::mirroring &1;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
		switch (regNum++ &3) {
		case 0:	chrOR =chrOR &~0x00FF |val;
			*EMU->multiCanSave =FALSE;
			break;
		case 1:	prgOR =prgOR &~0x00FF |val;
			break;
		case 2:	chrAND =0xFF >>(~val &0xF);
			chrOR =chrOR &~0x0F00 | ((val &0xF0) <<4);
			vrc4mode =!!(val &0x80);
			for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, vrc4mode? VRC24::write: MMC3::write);
			break;
		case 3:	prgAND =~val &0x3F;
			prgOR =prgOR &~0x0100 | ((val &0x40) <<2);
			chrOR =chrOR &~0x1000 | ((val &0x40) <<6);
			locked =!!(val &0x80);
			*EMU->multiCanSave =TRUE;
			break;
		}
	}
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	VRC24::load(sync, true, 0x05, 0x0A, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	vrc4mode =false;
	regNum =0;
	prgAND =0x3F; prgOR =0x00;
	chrAND =0xFF; chrOR =0x00;
	locked =false;
	VRC24::reset(resetType);
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_LONG(stateMode, offset, data, regNum);
	SAVELOAD_LONG(stateMode, offset, data, prgAND);
	SAVELOAD_LONG(stateMode, offset, data, prgOR);
	SAVELOAD_LONG(stateMode, offset, data, chrAND);
	SAVELOAD_LONG(stateMode, offset, data, chrOR);
	SAVELOAD_BOOL(stateMode, offset, data, locked);
	SAVELOAD_BOOL(stateMode, offset, data, vrc4mode);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, vrc4mode? VRC24::write: MMC3::write);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

void	MAPINT	cpuCycle () {
	if (vrc4mode)
		VRC24::cpuCycle();
	else
		MMC3::cpuCycle();
}

uint16_t mapperNum =556;
} // namespace


MapperInfo MapperInfo_556 = {
	&mapperNum,
	_T("C88DIP"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
