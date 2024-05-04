#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
FPPURead _PPURead3, _PPURead7;
uint8_t LatchState[2];

void	SyncCHR (void)
{
	int bank;

	bank = MMC3::getCHRBank(LatchState[0]? 2: 0) >>2;
	if (bank == 0)
		EMU->SetCHR_RAM4(0, 0);
	else	EMU->SetCHR_ROM4(0, bank);

	bank = MMC3::getCHRBank(LatchState[1]? 6: 4) >>2;
	if (bank == 0)
		EMU->SetCHR_RAM4(4, 0);
	else	EMU->SetCHR_ROM4(4, bank);
}

void	Sync (void) {
	MMC3::syncMirror();
	MMC3::syncPRG(0x3F, 0);
	MMC3::syncWRAM();
	SyncCHR();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data)
{
	offset = MMC3::saveLoad(mode, offset, data);
	for (int i = 0; i < 2; i++)
		SAVELOAD_BYTE(mode, offset, data, LatchState[i]);
	if (mode == STATE_LOAD)
		Sync();
	return offset;
}

int	MAPINT	PPURead3 (int Bank, int Addr)
{
	int result = _PPURead3(Bank, Addr);
	register int addy = Addr & 0x3F8;
	if (addy == 0x3D0)
	{
		LatchState[0] = 0;
		SyncCHR();
	}
	else if (addy == 0x3E8)
	{
		LatchState[0] = 1;
		SyncCHR();
	}
	return result;
}
int	MAPINT	PPURead7 (int Bank, int Addr)
{
	int result = _PPURead7(Bank, Addr);
	register int addy = Addr & 0x3F8;
	if (addy == 0x3D0)
	{
		LatchState[1] = 0;
		SyncCHR();
	}
	else if (addy == 0x3E8)
	{
		LatchState[1] = 1;
		SyncCHR();
	}
	return result;
}

BOOL	MAPINT	Load (void)
{
	MMC3::load(Sync);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	MMC3::reset(ResetType);

	_PPURead3 = EMU->GetPPUReadHandler(0x3);
	_PPURead7 = EMU->GetPPUReadHandler(0x7);

	EMU->SetPPUReadHandler(0x3, PPURead3);
	EMU->SetPPUReadHandler(0x7, PPURead7);
	EMU->SetPPUReadHandlerDebug(0x3, _PPURead3);
	EMU->SetPPUReadHandlerDebug(0x7, _PPURead7);
}

uint16_t MapperNum = 165;
} // namespace

MapperInfo MapperInfo_165 =
{
	&MapperNum,
	_T("Fire Emblem"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	SaveLoad,
	NULL,
	NULL
};