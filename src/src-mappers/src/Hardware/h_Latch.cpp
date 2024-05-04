#include	"h_latch.h"

namespace Latch {
uint8_t		data;
uint16_t	addr;
FSync		sync;
bool		busConflicts;

void	MAPINT	write (int bank, int _addr, int val) {
	if (busConflicts) val &=EMU->GetCPUReadHandler(bank)(bank, _addr);
	data =val;
	addr =bank <<12 |_addr;
	sync();
}

void	MAPINT	load (FSync _sync, bool _busConflicts) {
	busConflicts =_busConflicts;
	sync =_sync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		data =0;
		addr =0;
	}
	sync();
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
}

void	MAPINT	resetHard (RESET_TYPE resetType) {
	reset(RESET_HARD);
}

int	MAPINT	saveLoad_AD (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_WORD(stateMode, offset, _data, addr);
	SAVELOAD_BYTE(stateMode, offset, _data, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	saveLoad_AL (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_BYTE(stateMode, offset, _data, addr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	saveLoad_A (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_WORD(stateMode, offset, _data, addr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	saveLoad_D (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_BYTE(stateMode, offset, _data, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace Latch