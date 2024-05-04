#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace
{
void	Sync (void)
{
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr & 0x20)
		EMU->SetPRG_ROM32(0x8, (Latch::addr & 0x1E) >> 1);
	else
	{
		EMU->SetPRG_ROM16(0x8, Latch::addr & 0x1E);
		EMU->SetPRG_ROM16(0xC, Latch::addr & 0x1E);
	}
	switch ((Latch::addr & 0xC0) >> 6)
	{
	case 0:	EMU->Mirror_S0();		break;
	case 1:	EMU->Mirror_V();		break;
	case 2:	EMU->Mirror_H();		break;
	case 3:	EMU->Mirror_Custom(0, 1, 1, 1);	break;
	}
}

BOOL	MAPINT	Load (void)
{
	Latch::load(Sync, FALSE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	Latch::reset(ResetType);
}

uint16_t MapperNum = 231;
} // namespace

MapperInfo MapperInfo_231 =
{
	&MapperNum,
	_T("20-in-1"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};