#include "stdafx.h"
#include <stdint.h>

const static int16_t stepTable[16][16] ={
	{0,	14,	28,	42,	56,	70,	84,	97,	-111,	-97,	-84,	-70,	-56,	-42,	-28,	-14 },
	{0,	13,	26,	39,	52,	65,	78,	91,	-104,	-91,	-78,	-65,	-52,	-39,	-26,	-13 },
	{0,	11,	21,	32,	43,	54,	64,	75,	-86,	-75,	-64,	-54,	-43,	-32,	-21,	-11 },
	{0,	9,	18,	27,	35,	44,	53,	62,	-71,	-62,	-53,	-44,	-35,	-27,	-18,	-9  },
	{0,	7,	13,	20,	27,	34,	40,	47,	-54,	-47,	-40,	-34,	-27,	-20,	-13,	-7  },
	{0,	6,	11,	17,	22,	28,	33,	39,	-44,	-39,	-33,	-28,	-22,	-17,	-11,	-6  },
	{0,	5,	9,	14,	18,	23,	27,	32,	-36,	-32,	-27,	-23,	-18,	-14,	-9,	-5  },
	{0,	4,	8,	11,	15,	19,	23,	26,	-30,	-26,	-23,	-19,	-15,	-11,	-8,	-4  },
	{0,	3,	6,	9,	12,	15,	17,	20,	-23,	-20,	-17,	-15,	-12,	-9,	-6,	-3  },
	{0,	2,	5,	7,	10,	12,	14,	17,	-19,	-17,	-14,	-12,	-10,	-7,	-5,	-2  },
	{0,	2,	4,	6,	8,	10,	12,	14,	-16,	-14,	-12,	-10,	-8,	-6,	-4,	-2  },
	{0,	2,	3,	5,	6,	8,	10,	11,	-13,	-11,	-10,	-8,	-6,	-5,	-3,	-2  },
	{0,	1,	2,	4,	5,	6,	7,	9,	-10,	-9,	-7,	-6,	-5,	-4,	-2,	-1  },
	{0,	1,	2,	3,	4,	5,	6,	7,	-8,	-7,	-6,	-5,	-4,	-3,	-2,	-1  },
	{0,	1,	2,	3,	3,	4,	5,	6,	-7,	-6,	-5,	-4,	-3,	-3,	-2,	-1  },
	{0,	1,	1,	2,	3,	4,	4,	5,	-6,	-5,	-4,	-4,	-3,	-2,	-1,	-1  }
};

#define adpcm_lead       adpcmData[0]
#define adpcm_frame      adpcmData[1]
#define adpcm_volume     adpcmData[2]
#define adpcm_lastOutput adpcmData[3]
#define adpcm_output     adpcmData[4]
#define adpcm_position   adpcmData[5]

int32_t	vt369DecodeADPCM (uint8_t* adpcmData) {
	adpcm_position %=48;
	int nibble =adpcm_frame >>(adpcm_position &1? 4: 0);
	int index  =adpcm_lead
		   -(adpcm_position >=24 && adpcm_lead &0x40? 1: 0)
		   +(adpcm_position >=24 && adpcm_lead &0x80? 2: 0);
	int step   =stepTable[index &0xF][nibble &0xF];
	int8_t output =adpcm_output;
	switch (adpcm_lead >>4 &3) {
		case 0:	output =step;
			break;
		case 1:	output =step +(int8_t) adpcm_output;
			break;
		case 2: output =step +(int8_t) adpcm_output*2 - (int8_t) adpcm_lastOutput;
			break;
		case 3: output =step +(int8_t) adpcm_output   -((int8_t) adpcm_lastOutput >>1);
			break;
	}
	adpcm_lastOutput =adpcm_output;
	adpcm_output =output;
	adpcm_position++;
	return output *(adpcm_volume &0x7F);
}
