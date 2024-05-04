#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg[3];
uint8_t		a18;
uint8_t		lutIndex;
uint8_t		protIndex;
uint8_t		protValue;

void	sync (void) {
	MMC3::syncPRG(0x1F, a18 >>2);
	if (protIndex &0x20) {
		EMU->SetPRG_ROM8(0xA, prg[0] | a18 >>2);
		EMU->SetPRG_ROM8(0xC, prg[1] | a18 >>2);
		EMU->SetPRG_ROM8(0xE, prg[2] | a18 >>2);		
	}
	
	if (ROM->INES_PRGSize >256/16)
		MMC3::syncCHR_ROM(0xFF, a18 <<1);
	else
		MMC3::syncCHR_ROM(0xFF, ~MMC3::pointer <<1 &0x100, MMC3::pointer <<1 &0x100);
	
	MMC3::syncMirror();
}


int	MAPINT	readLUT (int bank, int addr) {
	const static uint8_t lut[8] ={ 0x83, 0x83, 0x42, 0x00, 0x00, 0x02, 0x02, 0x03 };
	return lut[lutIndex];
}

void	MAPINT	writeLUT (int bank, int addr, int val) {
	lutIndex =val &3 | addr >>6 &4;
	if (addr &0x100) {
		a18 =val &0x80;
		sync();
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr &3) {
		case 1: protValue =val <<5 &0x20 | val <<3 &0x10 | val <<1 &0x08 | val >>1 &0x04 | val >>3 &0x02 | val >>5 &0x01;
			if (protIndex ==0x26 ||
			    protIndex ==0x28 ||
			    protIndex ==0x2A)
				prg[0x15 -(protIndex >>1)] =protValue;
			break;
		case 3:	protIndex =val &0x3F;
			if (protIndex &0x20 && protValue) prg[2] =protValue;
			break;
	}
	MMC3::writeReg(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	MapperInfo_121.Description =(ROM->INES_PRGSize >256/16)? _T("卡聖 A9713"): _T("卡聖 A9711");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: prg) c =0;
		a18 =0;
		lutIndex =0;
		protIndex =0;
		protValue =0;
	}
	MMC3::reset(resetType);
	
	EMU->SetCPUReadHandler     (0x5, readLUT);
	EMU->SetCPUReadHandlerDebug(0x5, readLUT);
	EMU->SetCPUWriteHandler    (0x5, writeLUT);
	
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, a18);
	SAVELOAD_BYTE(stateMode, offset, data, lutIndex);
	SAVELOAD_BYTE(stateMode, offset, data, protIndex);
	SAVELOAD_BYTE(stateMode, offset, data, protValue);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =121;
} // namespace

MapperInfo MapperInfo_121 ={
	&mapperNum,
	_T("卡聖 A9711/A9713"),
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