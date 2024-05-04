#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		pointer;
uint8_t		prg[4];
uint8_t		chr[8];
	
void	sync (void) {
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 | bank <<1, prg[bank]);
	for (int bank =0; bank <8; bank++) iNES_SetCHR_Auto1(bank, chr[bank]);
	EMU->SetPRG_RAM8(0x6, 0);
	MMC3::syncMirror();
}

void	MAPINT	writeBank (int bank, int addr, int val) {
	if (~addr &1)
		pointer =val;
	else 
	switch(pointer) {
		case 0x00: chr[0] =val &0xFE;
		           chr[1] =val |0x01;
			   break;
		case 0x01: chr[2] =val &0xFE;
		           chr[3] =val |0x01;
			   break;
		case 0x02: chr[4] =val; break;
		case 0x03: chr[5] =val; break;
		case 0x04: chr[6] =val; break;
		case 0x05: chr[7] =val; break;
		case 0x06: prg[0] =val; break;
		case 0x07: prg[1] =val; break;
		case 0x46: prg[2] =val; break;
		case 0x47: prg[1] =val; break;
		case 0x80: chr[4] =val &0xFE;
		           chr[5] =val |0x01; break;
		case 0x81: chr[6] =val &0xFE;
		           chr[7] =val |0x01; break;
		case 0x82: chr[0] =val; break;
		case 0x83: chr[1] =val; break;
		case 0x84: chr[2] =val; break;
		case 0x85: chr[3] =val; break;
	}
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	for (int bank =0; bank <4; bank++) prg[bank] =(bank &2)? (0xFC |bank): bank;
	for (int bank =0; bank <8; bank++) chr[bank] =bank;
	MMC3::reset(ResetType);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeBank);

	// Copy trainer to RAM at $7000
	if (ROM->MiscROMSize && ROM->MiscROMData) for (size_t i =0; i <ROM->MiscROMSize; i++) {
		(EMU->GetCPUWriteHandler(0x7))(0x7, i, ROM->MiscROMData[i]);
		if (ROM->MiscROMData[0] ==0x4C) EMU->SetPC(0x7000);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, pointer);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =100;
} // namespace

MapperInfo MapperInfo_100 = {
	&mapperNum,
	_T("Nesticle MMC3"),
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

