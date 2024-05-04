#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace
{
void	Sync (void)
{
	if (Latch::addr & 0x1E)
	{
		EMU->SetPRG_ROM16(0x8, Latch::addr & 0x1F);
		EMU->SetPRG_ROM16(0xC, Latch::addr & 0x1F);
	}
	else	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, Latch::addr & 0x1F);
	if (Latch::addr & 0x20)
		EMU->Mirror_H();
	else	EMU->Mirror_V();
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

uint16_t MapperNum = 229;
} // namespace

MapperInfo MapperInfo_229 =
{
	&MapperNum,
	_T("SC 0892"),
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