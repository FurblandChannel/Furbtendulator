#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\h_Mic.h"

namespace
{
float	peak;

void	Sync (void)
{
	if (Latch::data & 0x10)
		EMU->SetPRG_ROM16(0x8, Latch::data & 0x7);
	else	EMU->SetPRG_ROM16(0x8, Latch::data | 0x8);
	EMU->SetPRG_ROM16(0xC, 0x7);
	EMU->SetCHR_RAM8(0, 0);
	peak = Mic::Read();
}

int	MAPINT	Read67 (int Bank, int Addr)
{
	return 3 | ((peak >0.333)? 4: 0);
}

BOOL	MAPINT	Load (void)
{
	Latch::load(Sync, FALSE);
	Mic::Load();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	iNES_SetMirroring();
	Latch::reset(ResetType);
	EMU->SetCPUReadHandler(0x6, Read67);
	EMU->SetCPUReadHandler(0x7, Read67);
	// this is debug-safe
	EMU->SetCPUReadHandlerDebug(0x6, Read67);
	EMU->SetCPUReadHandlerDebug(0x7, Read67);
}
void	MAPINT	Unload (void)
{
	Mic::Unload();
}

void	MAPINT	PPUCycle (int Addr, int Scanline, int Cycle, int IsRendering) {
	if (Scanline ==0 && Cycle==0) {
		peak =Mic::Read();
		/*TCHAR temp[16];
		_swprintf(temp, L"%f", peak);
		EMU->DbgOut(temp);*/
	}
}

uint16_t MapperNum = 188;
} // namespace

MapperInfo MapperInfo_188 =
{
	&MapperNum,
	_T("Bandai Karaoke Studio"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	PPUCycle,
	Latch::saveLoad_D,
	NULL,
	NULL
};