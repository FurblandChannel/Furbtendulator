#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

/*
	Submapper 1: Responds to $F000-$FFFF only, banked PRG-ROM + unbanked CHR-RAM
	Submapper 2: Responds to $E000-$FFFF only, banked PRG-ROM + banked CHR-ROM
	Submapper 3: Responds in $8000-$FFFF range, banked PRG-ROM + unbanked CHR-RAM
	Submapper 4: Responds in $8000-$FFFF range, unbanked PRG-ROM + banked CHR-ROM
*/

namespace {
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, ROM->INES2_SubMapper ==4? 0xFF: reg);
	EMU->SetPRG_ROM32(0x8, 0xFF);
	if (ROM->CHRROMSize)
		EMU->SetCHR_ROM8(0x0, reg);
	else
		EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (ROM->INES2_SubMapper ==1 && bank ==0xF ||
	    ROM->INES2_SubMapper ==2 && bank >=0xE ||
	    ROM->INES2_SubMapper >=3) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper <1 || ROM->INES2_SubMapper >4) {
		if (ROM->CHRROMSize ==0) {
			if (ROM->INES_Flags &1) // Vertical mirroring
				ROM->INES2_SubMapper =3;
			else
				ROM->INES2_SubMapper =1;
		} else {
			if (ROM->PRGROMSize <=32768)
				ROM->INES2_SubMapper =4;
			else
				ROM->INES2_SubMapper =2;
		}
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	sync();
	
	FDSsound::ResetBootleg(resetType);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =108;
} // namespace

MapperInfo MapperInfo_108 ={
	&MapperNum,
	_T("DH-08"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL,
};

