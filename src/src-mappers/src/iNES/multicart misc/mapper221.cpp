#include	"..\..\DLL\d_iNES.h"

namespace {
int	OuterBank;
uint8_t	InnerBank;
bool	nrom128;
bool	unrom;
bool	horizontalMirroring;
bool	protectCHR;

void	Sync (void) {
	if (nrom128) {
		EMU->SetPRG_ROM16(0x8, OuterBank | InnerBank);
		EMU->SetPRG_ROM16(0xC, OuterBank | InnerBank);
	} else {
		if (unrom) {
			EMU->SetPRG_ROM16(0x8, OuterBank | InnerBank);
			EMU->SetPRG_ROM16(0xC, OuterBank |         7);
		} else {
			EMU->SetPRG_ROM16(0x8, OuterBank | InnerBank &~1);
			EMU->SetPRG_ROM16(0xC, OuterBank | InnerBank | 1);
		}			
	}
	EMU->SetCHR_RAM8(0, 0);
	if (protectCHR) for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, (EMU->GetCHR_Ptr1(bank)), FALSE);
	if (horizontalMirroring) {
		EMU->Mirror_H();
	} else {
		EMU->Mirror_V();
	}
}

void	MAPINT	WriteOuterBank (int Bank, int Addr, int Val) {
	OuterBank = ((Addr &0xE0) >>2) | ((Addr &0x200) >>3);
	horizontalMirroring =!!(Addr &1);
	nrom128 =!(Addr &2);
	unrom =!!(Addr &0x100);
	Sync();
}

void	MAPINT	WriteInnerBank (int Bank, int Addr, int Val) {
	protectCHR =!!(Addr &8);
	InnerBank = Addr &7;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	for (int i =0x8; i <=0xB; i++) EMU->SetCPUWriteHandler(i, WriteOuterBank);
	for (int i =0xC; i <=0xF; i++) EMU->SetCPUWriteHandler(i, WriteInnerBank);
	if (ResetType ==RESET_HARD) {
		OuterBank =0;
		InnerBank =0;
		nrom128 =true;
		unrom =false;
		horizontalMirroring =false;
		protectCHR =false;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_WORD(mode, offset, data, OuterBank);
	SAVELOAD_BYTE(mode, offset, data, InnerBank);
	SAVELOAD_BOOL(mode, offset, data, nrom128);
	SAVELOAD_BOOL(mode, offset, data, unrom);
	SAVELOAD_BOOL(mode, offset, data, horizontalMirroring);
	SAVELOAD_BOOL(mode, offset, data, protectCHR);
	if (mode == STATE_LOAD)	Sync();
	return offset;
}

uint16_t MapperNum = 221;
} // namespace

MapperInfo MapperInfo_221 = {
	&MapperNum,
	_T("N625092"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
