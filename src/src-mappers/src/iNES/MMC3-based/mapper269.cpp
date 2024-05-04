#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
int	regNum;
int	prgAND, prgOR;
int	chrAND, chrOR;
bool	locked;

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR_ROM(chrAND, chrOR);
	MMC3::syncMirror();

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
	
	if (ROM->CHRROMSize ==0) {
		EMU->SetCHRROMSize(ROM->PRGROMSize);
		// Decrypt the CHR pattern data. Since we do not know which part of the one-bus PRG-ROM will be mapped into PPU address space, decrypt the entire ROM, and use that as our CHR-ROM.
		for (unsigned int i =0; i <ROM->PRGROMSize; i++) {
			uint8_t Val =ROM->PRGROMData[i];
			Val =((Val &1) <<6) | ((Val &2) <<3) | ((Val &4) <<0) | ((Val &8) >>3) | ((Val &16) >>3) | ((Val &32) >>2) | ((Val &64) >>1) | ((Val &128) <<0);
			ROM->CHRROMData[i] =Val;
		}
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	regNum =0;
	prgAND =0x3F; prgOR =0x00;
	chrAND =0xFF; chrOR =0x00;
	locked =false;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_LONG(stateMode, offset, data, regNum);
	SAVELOAD_LONG(stateMode, offset, data, prgAND);
	SAVELOAD_LONG(stateMode, offset, data, prgOR);
	SAVELOAD_LONG(stateMode, offset, data, chrAND);
	SAVELOAD_LONG(stateMode, offset, data, chrOR);
	SAVELOAD_BOOL(stateMode, offset, data, locked);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =269;
} // namespace


MapperInfo MapperInfo_269 = {
	&mapperNum,
	_T("Games Xplosion 121-in-1"),
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
