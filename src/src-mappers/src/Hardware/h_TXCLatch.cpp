#include "h_TXCLatch.h"

namespace TXCLatch {
bool		increase;
uint8_t		output;
uint8_t		invert;
uint8_t		staging;
uint8_t		accumulator;
uint8_t		inverter;
bool		A, B, X, Y;
FSync		sync;

int	MAPINT	read (int bank, int addr) {
	uint8_t	result =*EMU->OpenBus;
	if ((addr &0x103) ==0x100) {
		result =accumulator &0x7 | (inverter ^invert) &~0x7;
		Y =X | !!(result &0x10);	
		sync();
	}
	return result;
}

void	MAPINT	write (int bank, int addr, int val) {
	if (bank &0x8)
		output =accumulator &0xF | inverter <<1 &0x10;
	else
	switch (addr &0x103) {
		case 0x100:
			if (increase)
				accumulator++;
			else
				accumulator =accumulator &~0x7 |(staging ^invert) &0x7;
			break;
		case 0x101:
			invert =val &1? 0xFF: 0x00;
			break;
		case 0x102:
			staging =val &0x7;
			inverter =val &~0x7;
			break;
		case 0x103:
			increase =!!(val &1);
			break;
	}
	X =invert? A: B;
	Y =X | !!(val &0x10);
	sync();
}

void	MAPINT	load (FSync _sync) {
	sync =_sync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		output =0;
		invert =0;
		staging =0;
		accumulator =0;
		inverter =0;
		A =0;
		B =1;
		X =A;
		Y =X;
	}
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, increase);
	SAVELOAD_BYTE(stateMode, offset, data, output);
	SAVELOAD_BYTE(stateMode, offset, data, invert);
	SAVELOAD_BYTE(stateMode, offset, data, staging);
	SAVELOAD_BYTE(stateMode, offset, data, accumulator);
	SAVELOAD_BYTE(stateMode, offset, data, inverter);
	SAVELOAD_BOOL(stateMode, offset, data, A);
	SAVELOAD_BOOL(stateMode, offset, data, B);
	SAVELOAD_BOOL(stateMode, offset, data, X);
	SAVELOAD_BOOL(stateMode, offset, data, Y);
	return offset;
}
};
