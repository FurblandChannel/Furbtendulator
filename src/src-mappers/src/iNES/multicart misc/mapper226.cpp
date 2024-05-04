#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	Reg[2];
FCPURead _Read;

void	Sync (void) {
	uint8_t OuterBank =(Reg[0] >>7) | ((Reg[1] &1) <<1);
	if (ROM->INES_PRGSize ==1536/16 && OuterBank >0)	// Mapper 226 with 1536 KiB: Outer bank order 0 0 1 2
		OuterBank--;
	if (Reg[0] &0x20) {	// NROM-128
		EMU->SetPRG_ROM16(0x8, (OuterBank <<5) | (Reg[0] &0x1F));
		EMU->SetPRG_ROM16(0xC, (OuterBank <<5) | (Reg[0] &0x1F));
	} else			// NROM-256
		EMU->SetPRG_ROM32(0x8,((OuterBank <<5) | (Reg[0] &0x1E)) >>1);
	EMU->SetCHR_RAM8(0, 0);
	
	// CHR-RAM protect
	if (Reg[1] &0x02) for (int i =0; i <8; i++) EMU->SetCHR_Ptr1(i, EMU->GetCHR_Ptr1(i), FALSE);
	
	// Mirroring
	if (Reg[0] &0x40)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Reg[Addr &1] =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	Reg[0] =Reg[1] =0;
	for (int i =0x8; i <=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Reg[0]);
	SAVELOAD_BYTE(mode, offset, data, Reg[1]);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =226;
} // namespace

MapperInfo MapperInfo_226 ={
	&MapperNum,
	_T("0380/910307"),
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