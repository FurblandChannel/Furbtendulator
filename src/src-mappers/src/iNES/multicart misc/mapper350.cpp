#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		outerBank;
uint8_t		innerBank;
bool		locked;

void	sync (void) {
	int bank =outerBank &0x18 | innerBank &0x07;

	EMU->SetPRG_ROM8(0x6, 1);
	switch(outerBank >>5 &3) {
		case 0:	EMU->SetPRG_ROM16(0x8, bank);
			EMU->SetPRG_ROM16(0xC, bank);
			break;
		case 1:	EMU->SetPRG_ROM32(0x8, bank >>1);
			break;
		default:if (outerBank &0x20) bank &=0x07; // Second chip only has 128 KiB
			EMU->SetPRG_ROM16(0x8, bank | outerBank &0x20      );
			EMU->SetPRG_ROM16(0xC, bank | outerBank &0x20 |0x07);
			if (outerBank &0x20 && ROM->PRGROMSize <=512*1024) for (bank =0x8; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
			break;
	}
	
	EMU->SetCHR_RAM8(0x0, 0);
	for (bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), !!(outerBank &0x40));
	
	if (outerBank &0x40)
		iNES_SetMirroring();
	else
	if (outerBank &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (!locked) {
		locked =!!(bank &2);
		outerBank =val;
		sync();
	}
}

void	MAPINT	writeInnerBank (int bank, int addr, int val) {
	if (!locked) {
		innerBank =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		outerBank =0;
		innerBank =0;
		locked =false;
	}
	sync();
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeInnerBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, innerBank);
	SAVELOAD_BOOL(stateMode, offset, data, locked);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =350;
} // namespace

MapperInfo MapperInfo_350 = {
	&MapperNum,
	_T("891227"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};