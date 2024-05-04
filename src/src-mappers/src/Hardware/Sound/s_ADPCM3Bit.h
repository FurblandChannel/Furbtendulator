#pragma once
#include <stdint.h>
#include <queue>
#include <vector>
#include "../../interface.h"

class ADPCM3Bit {
	uint32_t             count;         // counter until next decoded sample is output
	uint32_t             period;        // counter target until next decoded sample is output
	uint8_t	             command;       // current command
	uint8_t              latch;         // latch for loading the next command
	uint8_t              data;          // 4-bit data
	uint8_t              bytesLeft;     // bytes left to receive for the current command
	bool                 clock;         // data clock signal
	bool                 ready;         // indicates that frame buffer is not full
	uint8_t              index;         // index into ADPCM table
	int8_t               output;        // current decoded ADPCM output
	std::vector<uint8_t> input;         // buffer of input bytes
	std::queue<int64_t>  frames;        // buffer of input frames, at most 12
	
	void                 decodeSample(uint8_t);
public:
		   ADPCM3Bit ();
	void       reset     ();
	void       run       ();
        bool       getClock  () const;
	int8_t     getOutput () const;
	uint32_t   getPeriod () const;
	bool       getReady  () const;
	void       setClock  (bool);
	void       setData   (uint8_t);
	int MAPINT saveLoad  (STATE_TYPE, int, unsigned char *);
};