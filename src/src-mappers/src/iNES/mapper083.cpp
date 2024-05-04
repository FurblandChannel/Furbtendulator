#include "..\DLL\d_iNES.h"

#define	mirroring         misc &0x03
#define	prgMode           misc >>3 &3
#define	mapWROM           misc &0x20
#define	decreasing        misc &0x40
#define	counterEnabled    misc &0x80

namespace {
uint8_t		outerBank;
uint8_t		misc;

uint8_t		prgMask;
uint8_t		prg[4];

uint8_t		chrMode;
uint8_t		chr[8];

uint16_t	dipMask;
uint8_t		scratch[4];

uint16_t	counter;
bool		counting;

bool		mapWRAM;

void	sync (void) {
	switch (prgMode) {
		case 0:	EMU->SetPRG_ROM16(0x8, outerBank              );
			EMU->SetPRG_ROM16(0xC, outerBank | prgMask >>1);
			break;
		case 1:	EMU->SetPRG_ROM32(0x8, outerBank >>1);
			break;
		case 2:
		case 3:	EMU->SetPRG_ROM8 (0x8, outerBank <<1 &~prgMask | prg[0] &prgMask);
			EMU->SetPRG_ROM8 (0xA, outerBank <<1 &~prgMask | prg[1] &prgMask);
			EMU->SetPRG_ROM8 (0xC, outerBank <<1 &~prgMask | prg[2] &prgMask);
			EMU->SetPRG_ROM8 (0xE, outerBank <<1 &~prgMask | 0x1F   &prgMask);
			break;
	}
	if (mapWRAM)
		EMU->SetPRG_RAM8(0x6, outerBank >>6);
	else
	if (mapWROM)
		EMU->SetPRG_ROM8(0x6, prg[3]);
	else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}

	switch (chrMode) {
		default:
		case 0:	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
			break;
		case 1:	EMU->SetCHR_ROM2(0, chr[0]);
			EMU->SetCHR_ROM2(2, chr[1]);
			EMU->SetCHR_ROM2(4, chr[6]);
			EMU->SetCHR_ROM2(6, chr[7]);
			break;
		case 2:	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank] | outerBank <<4 &0x300);
			break;
	}
	switch (mirroring) {
		case 0:	EMU->Mirror_V();  break;
		case 1:	EMU->Mirror_H();  break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

int	MAPINT	readScratchDIP (int bank, int addr) {
	if (addr &dipMask)
		return scratch[addr &3];	
	else
		return ROM->dipValue;
}

void	MAPINT	writeScratch (int bank, int addr, int val) {
	scratch[addr &3] =val;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (ROM->INES_MapperNum ==264)	// 264 shifts the register number by two bits to the right
		addr =addr >>2 &0x3FC0 | addr &0x003F;
		
	int reg   =addr >>8 &3;
	int index =addr &0x1F;
	switch (reg) {
		case 0:	outerBank =val;
			break;
		case 1:	misc =val;
			break;
		case 2:	if (index &1) {
				counter =counter &0x00FF | val <<8;
				counting=counterEnabled;
			} else {
				counter =counter &0xFF00 | val;
				EMU->SetIRQ(1);
			}
			break;
		case 3:	if (index <0x10)
				prg[index &3] =val;
			else
			if (index <0x18)
				chr[index &7] =val;
	}
	sync();
}

BOOL	MAPINT	load083 (void) {
	iNES_SetSRAM();
	if (ROM->INES_Version <2) {
		if (ROM->CHRROMSize >=1024*1024) { ROM->INES2_SubMapper =2; EMU->Set_SRAMSize(32768); } else
		if (ROM->CHRROMSize >= 512*1024)   ROM->INES2_SubMapper =1;
	}
	prgMask =0x1F;
	mapWRAM =ROM->INES2_SubMapper ==2;
	chrMode =ROM->INES2_SubMapper;
	dipMask =0x100;
	return TRUE;
}

BOOL	MAPINT	load264 (void) {
	iNES_SetSRAM();
	prgMask =0x0F;
	mapWRAM =false;
	chrMode =1;
	dipMask =0x400;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType == RESET_HARD) {
		outerBank =0;
		misc =2 <<3;
		for (int bank =0; bank <4; bank++) prg[bank] =0xFC |bank;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		for (int bank =0; bank <4; bank++) scratch[bank] =0;		
		counter =0;
		counting =false;
	}
	sync();

	EMU->SetCPUReadHandler(0x5, readScratchDIP);
	EMU->SetCPUWriteHandler(0x5, writeScratch);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (counting && counter) {
		if (decreasing)
			counter--;
		else	
			counter++;
		if (!counter) {
			EMU->SetIRQ(0);
			counting =false;
		}
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, misc);
	for (auto c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);	
	for (auto c: scratch) SAVELOAD_BYTE(stateMode, offset, data, c);	
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BOOL(stateMode, offset, data, counting);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum083 =83;
uint16_t mapperNum264 =264;
} // namespace

MapperInfo MapperInfo_083 = {
	&mapperNum083,
	_T("Cony"),
	COMPAT_FULL,
	load083,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_264 = {
	&mapperNum264,
	_T("Yoko"),
	COMPAT_FULL,
	load264,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
