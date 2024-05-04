#pragma once
#include "..\interface.h"

class FlashROM {
protected:
	const uint8_t	manufacturerID;
	const uint8_t	modelID;
	int		state;
	uint8_t* const	data;
	const size_t	chipSize;
	const size_t	sectorSize;
	int             timeOut;
	const uint16_t	magicAddr1;
	const uint16_t	magicAddr2;
public:
	FlashROM      (uint8_t, uint8_t, uint8_t*, size_t, size_t);
	FlashROM      (uint8_t, uint8_t, uint8_t*, size_t, size_t, uint16_t, uint16_t);
	int read      (int, int) const;
	void write    (int, int, int);
	void cpuCycle ();
};