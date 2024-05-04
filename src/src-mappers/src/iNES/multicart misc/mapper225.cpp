#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t	Scratch[4];

void	Sync (void) {
	if (Latch::addr &0x1000) {
		EMU->SetPRG_ROM16(0x8, (Latch::addr >>6) &0x3F | ((Latch::addr &0x4000)? 0x40: 0x00) );
		EMU->SetPRG_ROM16(0xC, (Latch::addr >>6) &0x3F | ((Latch::addr &0x4000)? 0x40: 0x00) );
	} else
		EMU->SetPRG_ROM32(0x8, (Latch::addr >>7) &0x1F | ((Latch::addr &0x4000)? 0x20: 0x00) );
	EMU->SetCHR_ROM8(0x0, (Latch::addr &0x3F) | ((Latch::addr &0x4000)? 0x40: 0x00) );
	if (Latch::addr &0x2000)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	Read5 (int Bank, int Addr) {
	if (Addr &0x800)
		return Scratch[Addr &3];
	else
		return *EMU->OpenBus;
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	if (Addr &0x800)
		Scratch[Addr &3] =Val &0xF;
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
	EMU->SetCPUReadHandler(5, Read5);
	EMU->SetCPUWriteHandler(5, Write5);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, Scratch[i]);
	offset = Latch::saveLoad_A(mode, offset, data);
	
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum225 = 225;
uint16_t MapperNum255 = 255;
} // namespace

MapperInfo MapperInfo_225 = {
	&MapperNum225,
	_T("ET-4310/K-1010"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_255 = {
	&MapperNum255,
	_T("ET-4310/K-1010"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};