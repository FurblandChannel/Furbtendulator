#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace
{
uint8_t prot;
FCPURead _Read4;
FCPUWrite _Write4;

static uint8_t lut[4] = { 0x00, 0x02, 0x02, 0x03 };

void	Sync (void) {
	MMC3::syncPRG(0xFF,0x00);
	MMC3::syncCHR_ROM(0xFF,0x00);
	MMC3::syncMirror();
}
int	MAPINT	Read (int Bank, int Addr) {
	if (Bank==4 && Addr <0x20)
		return _Read4(Bank, Addr);
	else
		return prot;
}


void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Bank==4 && Addr <0x20)
		_Write4(Bank, Addr, Val);
	else
		prot = lut[Val &3];
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	MMC3::reset(ResetType);
	_Read4 = EMU->GetCPUReadHandler(0x4);
	_Write4 = EMU->GetCPUWriteHandler(0x4);
	for (int i=4; i<8; i++) {
		EMU->SetCPUReadHandler(i, Read);
		EMU->SetCPUWriteHandler(i, Write);
	}
	if (ResetType == RESET_HARD) prot = 0;
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, prot);
	if (mode == STATE_LOAD)
		Sync();
	return offset;
}

uint16_t MapperNum = 238;
} // namespace

MapperInfo MapperInfo_238 =
{
	&MapperNum,
	_T("Sakano MMC3"),
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