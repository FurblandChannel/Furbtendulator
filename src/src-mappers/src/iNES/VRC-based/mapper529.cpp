#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\h_EEPROM_93Cx6.h"

namespace {
EEPROM_93Cx6*	eeprom =NULL;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, VRC24::prg[1]);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	VRC24::syncCHR(0x1FF, 0x00);
	VRC24::syncMirror();
}

int	MAPINT	readEEPROM (int bank, int addr) {
	if (eeprom)
		return eeprom->read()? 0x01: 0x00;
	else
		return 0x01;
}
// B24F: save
void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &0x800) {
		if (eeprom) eeprom->write(!!(addr &0x04), !!(addr &0x02), !!(addr &0x01));
	} else
	switch(bank) {
		case 0x8: case 0xA:
			VRC24::writePRG(bank, addr, val);
			break;
		case 0x9:
			VRC24::writeMisc(bank, addr, val);
			break;
		case 0xB: case 0xC: case 0xD: case 0xE:
			VRC24::writeCHR(bank, addr, val);
			break;
		case 0xF:
			VRC24::writeIRQ(bank, addr, val);
			break;
	}
}

BOOL	MAPINT	load (void) {
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	size_t sizeSave =(ROM->INES2_PRGRAM &0xF0)? (64 << (ROM->INES2_PRGRAM >> 4)): 0;
	if (sizeSave ==256) {
		iNES_SetSRAM();
		eeprom =new EEPROM_93Cx6(ROM->PRGRAMData, 256, 16);
	}
	MapperInfo_529.Description =ROM->CHRROMSize? _T("YY0807/J-2148"): _T("T-230");
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	VRC24::reset(resetType);
	if (eeprom) {
		EMU->SetCPUReadHandler(0x5, readEEPROM);
		for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	}
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

uint16_t mapperNum =529;
} // namespace

MapperInfo MapperInfo_529 = {
	&mapperNum,
	_T("YY0807/J-2148/T-230"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};
/*
18	byte to be stored
1B	address?
FE2C	store byte
	FFA0	Start Sequence
	FEAC	Store two bits
		FED3	Store X bits
	FED1	Store 8 bits
	
	FEB5	read 16 bits
	FEB9	read 8 bits
	
	FE4B	Store 16 bits?
	
	FE95	Write Misc Command
	FE72	
*/