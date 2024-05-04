#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define mixedCHR  !!(reg[4] &0x01)
#define mixedMask   (reg[4] &0xFE)
namespace {
uint8_t		reg[8];

void	sync (void) {
	if (MMC3::wram &0x20) MMC3::wram &=~0x40; // Hack for FS005 games on Mindkids board that only work with emulation.
	int prgMaskMMC3     = (reg[3] &0x10? 0x00: 0x0F) // PRG A13-A16
	                    | (reg[0] &0x40? 0x00: 0x10) // 128 KiB ^=PRG A17
	                    | (reg[1] &0x80? 0x00: 0x20) // 256 KiB ^=PRG A18
			    | (reg[1] &0x40? 0x40: 0x00) // 512 KiB ^=PRG A19
			    | (reg[1] &0x20? 0x80: 0x00) // 1 MiB   ^=PRG A20
	;
	int prgMaskGNROM;
	int prgOffset;
	bool chip;
	switch (ROM->INES2_SubMapper &~1) {
		default:
			prgMaskGNROM    = (reg[3] &0x10? (reg[1] &0x02? 0x03: 0x01): 0x00);
			prgOffset       = (reg[3] &0x0E)
					|((reg[0] &0x07) <<4)
					| (reg[1] &0x10? 0x80: 0x00)
					|((reg[1] &0x0C) <<6)
					|((reg[0] &0x30) <<6);
			break;
		case 2:	/* Different arrangement of register 1 */
			prgMaskGNROM    = (reg[3] &0x10? (reg[1] &0x10? 0x01: 0x03): 0x00);
			prgOffset       = (reg[3] &0x0E)
					|((reg[0] &0x07) <<4)
					| (reg[1] &0x08? 0x80: 0x00)
					| (reg[1] &0x04? 0x100: 0x00)
					| (reg[1] &0x02? 0x200: 0x00)
					|((reg[0] &0x30) <<6);
			break;
		case 4:	/* Different arrangement of register 1, PRG A20-A21 in register 0 */
			prgMaskGNROM    = (reg[3] &0x10? (reg[1] &0x02? 0x03: 0x01): 0x00);
			prgOffset       = (reg[3] &0x0E)
					|((reg[0] &0x07) <<4)
					|((reg[0] &0x30) <<3);
			break;
		case 6:	/* Like 0, but CHR A17 selects between two PRG ROM chips */
			prgMaskGNROM    = (reg[3] &0x10? (reg[1] &0x02? 0x03: 0x01): 0x00);
			prgOffset       = (reg[3] &0x0E)
					|((reg[0] &0x07) <<4)
					| (reg[1] &0x10? 0x80: 0x00)
					|((reg[1] &0x0C) <<6)
					|((reg[0] &0x30) <<6);
			prgOffset &=ROM->PRGROMSize /8192 /2 -1;
			if (reg[0] &0x80)
				chip =!!(reg[0] &0x08);
			else
				chip =!!(MMC3::getCHRBank(0) &0x80);
			if (chip) prgOffset |=ROM->PRGROMSize /8192 /2;				
			break;
	}
	int maskedPRGOffset = prgOffset &~(prgMaskMMC3 | prgMaskGNROM);
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, MMC3::getPRGBank(bank) &prgMaskMMC3 | maskedPRGOffset | bank&prgMaskGNROM);
	
	int chrMaskMMC3     =  reg[3] &0x10? 0x00: reg[0] &0x80? 0x7F: 0xFF;
	int chrOffset       =  reg[2] <<3 &0x78 | reg[0] <<4 &0x80;
	int chrMaskGNROM    =  reg[3] &0x10? 0x07: 0x00;
	switch (ROM->INES2_SubMapper &~1) {
		default:
			chrOffset |=reg[0] <<4 &0x300;
			break;
		case 4:
			chrOffset |=reg[0] <<7 &0x300 | reg[0] <<6 &0xC00;
			break;
	}
	int maskedCHROffset = chrOffset &~(chrMaskMMC3 | chrMaskGNROM);	
	for (int bank =0; bank <8; bank++) {
		if (mixedCHR && (MMC3::getCHRBank(bank) &0xFE) ==mixedMask)
			EMU->SetCHR_RAM1(bank, MMC3::getCHRBank(bank) &chrMaskMMC3 | maskedCHROffset | bank&chrMaskGNROM);
		else
			iNES_SetCHR_Auto1(bank, MMC3::getCHRBank(bank) &chrMaskMMC3 | maskedCHROffset | bank&chrMaskGNROM);
	}
				   
	// Incomplete MMC4 mode
	if (reg[3] &0x40) {
		if (~MMC3::pointer &0x40) {
			EMU->SetPRG_ROM8(0xC, maskedPRGOffset |(2 &prgMaskGNROM));
			EMU->SetPRG_ROM8(0xE, maskedPRGOffset |(3 &prgMaskGNROM));
		}
		EMU->SetCHR_RAM1(0x0 ^(MMC3::pointer &0x80? 4: 0), MMC3::reg[0] &chrMaskMMC3 |maskedCHROffset | 0&chrMaskGNROM);
		EMU->SetCHR_RAM1(0x1 ^(MMC3::pointer &0x80? 4: 0), 0x00         &chrMaskMMC3 |maskedCHROffset | 1&chrMaskGNROM);
		EMU->SetCHR_RAM1(0x2 ^(MMC3::pointer &0x80? 4: 0), MMC3::reg[1] &chrMaskMMC3 |maskedCHROffset | 2&chrMaskGNROM);
		EMU->SetCHR_RAM1(0x3 ^(MMC3::pointer &0x80? 4: 0), 0x00         &chrMaskMMC3 |maskedCHROffset | 3&chrMaskGNROM);
	}
	
	MMC3::syncMirror();
	MMC3::syncWRAM();
	if ((ROM->INES2_SubMapper &~1) ==8 && reg[0] &0x10) protectCHRRAM();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank >=0x6) MMC3::wramWrite(bank, addr, val);
	
	if (~reg[3] &0x80 || (addr &7) ==2) {
		if ((addr &7) ==2) {
			if (reg[2] &0x80) val =val &0x0F | reg[2] &~0x0F;
			val &=~reg[2] >>3 &0xE |0xF1;
		}
		reg[addr &7] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync);
	if (ROM->INES_MapperNum ==268) {
		if ((ROM->INES2_SubMapper &~1)==0 && ROM->CHRROMSize >256*1024)
			MapperInfo_268.Description =_T("JTH-813");
		else
		switch(ROM->INES2_SubMapper) {
		default:
		case 0: MapperInfo_268.Description =_T("Coolboy"); break;
		case 1: MapperInfo_268.Description =_T("Mindkids"); break;
		case 2:
		case 3: MapperInfo_268.Description =_T("Mindkids SMD172C-L1"); break;
		case 4:
		case 5:	MapperInfo_268.Description =_T("LD622D"); break;
		case 6:
		case 7:	MapperInfo_268.Description =_T("J-852C"); break;
		} 
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(RESET_HARD);
	if (ROM->INES_MapperNum ==224 || ROM->INES2_SubMapper &1)
		EMU->SetCPUWriteHandler(0x5, writeReg);
	else {
		EMU->SetCPUWriteHandler(0x6, writeReg);
		EMU->SetCPUWriteHandler(0x7, writeReg);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum224 =224;
uint16_t mapperNum268 =268;
} // namespace

MapperInfo MapperInfo_224 ={
	&mapperNum224,
	_T("Jncota KT-008"),
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

MapperInfo MapperInfo_268 ={
	&mapperNum268,
	_T("KP6022/AA6023 ASIC"),
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