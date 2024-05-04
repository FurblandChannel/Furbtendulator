#pragma once
#include "..\interface.h"

namespace TXCLatch {
extern	bool		increase;
extern	uint8_t		input;
extern	uint8_t		output;
extern	uint8_t		invert;
extern	uint8_t		staging;
extern	uint8_t		accumulator;
extern	uint8_t		inverter;
extern	bool		A, B, X, Y;
extern	FSync		sync;

int	MAPINT	read (int, int);
void	MAPINT	write (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char*);
};
