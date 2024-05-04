#include "stdafx.h"
#include <stdint.h>

static const uint8_t indexStep     [4] ={ 0,  0,  3,  5 };
static const uint8_t indexTable   [26] ={ 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 20, 20, 20 };
static const int8_t  stepTable [4][21] ={
		{  0,   1,   1,   1,   1,   1,   2,   2,   2,   3,   3,   4,   5,   5,   6,   7,   8,  10,  11,  13,  15 },
		{  1,   3,   3,   3,   4,   4,   6,   6,   7,   9,  10,  12,  15,  16,  19,  22,  25,  30,  34,  40,  46 },
		{  3,   5,   5,   6,   7,   8,  10,  11,  13,  16,  18,  21,  25,  28,  32,  38,  43,  51,  58,  68,  78 },
		{  4,   7,   7,   8,  10,  11,  14,  15,  18,  22,  25,  29,  35,  39,  45,  53,  60,  71,  81,  95, 109 }
};

void vt1682DecodeADPCM (uint8_t code, int8_t& output, uint8_t& index) {
	int16_t predictor =output +stepTable[code &3][index] *(code &4? -1: 1);
	output =predictor <-128? -128: predictor >127? 127: predictor;
	index =indexTable[index +indexStep[code &3]];
}