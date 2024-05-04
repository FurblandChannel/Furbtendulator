#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[4];

void	sync (void) {
	int prgAND = reg[0] &0x40? 0x0F: 0x1F;
	int prgOR  =(reg[0] <<4 &0x70 | (reg[0] ^0x20) <<3 &0x180) &~prgAND;
	if (ROM->INES2_SubMapper ==1) prgOR |=prgOR >>1 &0x80;    /* Submapper 1 uses PRG A21 as a chip select between two 1 MiB chips */
	if (ROM->INES2_SubMapper ==2) prgOR |=reg[1] <<5 &0x80;   /* Submapper 2 uses 6001.2 (not documented in datasheet) as a chip select between two 1 MiB chips */
	
	for (int bank =0; bank <4; bank++) {
		/* In UNROM-like mode (CT3=1, CT2=1, CT0=1), MMC3 sees A13=0 and A14=CPU A14 during reads, making register 6 apply from $8000-$BFFF, and the fixed bank from $C000-$FFFF.
		   In NROM-128, NROM-256, ANROM and UNROM modes (CT0=1), MMC3 sees A13=0 and A14=0, making register 6 apply from $8000-$FFFF. */
		int val =MMC3::getPRGBank(bank &((reg[3] &0x0D) ==0x0D? 2: reg[3] &0x01? 0: 3));
		
		/* UNROM and ANROM modes mean that MMC3 register 6 selects 16 and 32 KiB rather than 8 KiB banks. */
		if (reg[3] &0x08) switch(reg[3] &0x03) {
			case 0: val =val &3  | val <<1 &~3; break; /* PRG A14 appears twice */
			case 1: val =bank &3 | val <<1 &~1; break; /* 16 KiB mode, bit 0 OR'd with CPU A14 */
			case 2: val =val &3  | val <<2 &~3; break; /* PRG A13 and PRG A14 appear twice */
			case 3: val =bank &3 | val <<2 &~3; break; /* 32 KiB mode */
		} else
		if (reg[3] &0x01) { /* regular NROM modes */
			val =bank &1 | val &~1;
			if (reg[3] &0x02) val =bank &2 | val &~2; /* NROM-256 */
		}
		EMU->SetPRG_ROM8(0x8 +bank*2, val &prgAND | prgOR &~prgAND);		
	}
	EMU->SetPRG_RAM8(0x6, 0); /* Verified: WRAM cannot be disabled */

	int chrAND = reg[0] &0x80? 0x7F: 0xFF;
	int chrOR;
	if (ROM->INES_MapperNum ==126)
		chrOR =(reg[0] <<4 &0x080 | (reg[0] ^0x20) <<3 &0x100 | reg[0] <<5 &0x200) &~chrAND;
	else
		chrOR =(reg[0] ^0x20) <<4 &0x380 &~chrAND;
	
	if (reg[3] &0x10) /* CNROM CHR banking */
		iNES_SetCHR_Auto8(0x0, reg[2] &(chrAND >>3) | (chrOR &~chrAND) >>3);
	else              /* MMC3 CHR banking */
		MMC3::syncCHR(chrAND, chrOR);
	
	if (reg[3] &0x20) { /* ANROM mirroring */
		if (MMC3::reg[6] &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
	if (reg[1] &0x02) { /* Extended MMC3 mirroring */
		switch(MMC3::mirroring &3) {
		case 0: EMU->Mirror_V(); break;
		case 1: EMU->Mirror_H(); break;
		case 2: EMU->Mirror_S0(); break;
		case 3: EMU->Mirror_S1(); break;
		}
	} else              /* Normal MMC3 mirroring */
		MMC3::syncMirror();
}

int	MAPINT	interceptCartRead (int bank, int addr) {
	if (reg[1] &0x01)
		return EMU->ReadPRG(bank, addr &~1 | ROM->dipValue &1); /* Replace A0 with SL0 input */
	else
		return EMU->ReadPRG(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if ((addr &3) ==2) { /* CNROM Bank (D0-D3), Bank Enable (D4-D6) and Bank Enable Lock (D7) */
		int mask =0xFF &~(reg[2] &0x80? 0x70: 0x00) &~(reg [2] >>3 &0x0E);
		reg[2] =reg[2] &~mask | val &mask;
		sync();
	} else
	if (~reg[3] &0x80) { /* Lock registers except 6002 */
		reg[addr &3] =val;
		sync();
	}
}

void	MAPINT	writeCart (int bank, int addr, int val) {
	if (reg[3] &0x08) addr =1; /* A0 substitution only looks at bit 3 of register 3 */
	if ((reg[3] &0x09) ==0x09) /* UNROM and ANROM modes treat all writes to $8000-$FFFF as if they were going to $8000-$9FFF */
		MMC3::writeReg(0x8, addr, val); 
	else
		MMC3::write(bank, addr, val);
}

void	MAPINT	invertScanlineCounter (int bank, int addr, int val) {
	MMC3::writeIRQConfig(bank, addr, val ^0xFF);
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::NEC); /* Verified using blargg's MMC3 tests */
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, interceptCartRead);
		EMU->SetCPUWriteHandler(bank, writeCart);
	}
	EMU->SetCPUWriteHandler(0x6, writeReg);
	if (ROM->INES_MapperNum ==534) {
		EMU->SetCPUWriteHandler(0xC, invertScanlineCounter);
		EMU->SetCPUWriteHandler(0xD, invertScanlineCounter);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum126 =126;
uint16_t mapperNum534 =534;
} // namespace

MapperInfo MapperInfo_126 = {
	&mapperNum126,
	_T("TEC9719 with swapped CHR"),
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
MapperInfo MapperInfo_422 = {
	&mapperNum534,
	_T("TEC9719"),
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
MapperInfo MapperInfo_534 = {
	&mapperNum534,
	_T("ING003C"),
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