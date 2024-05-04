#include "vgmcapture.h"

#define CMD_YM2413_WRITE 0x51
#define CMD_NESAPU_WRITE 0xB4

#define CMD_WAIT_N_SAMPLES 0x61
#define CMD_WAIT_735_SAMPLES 0x62
#define CMD_WAIT_882_SAMPLES 0x63
#define CMD_DATA_BLOCK 0x67
#define CMD_WAIT_SHORT 0x70

#define CMD_END_OF_SOUND_DATA 0x66

/* Helper functions */
void static inline put_lsb16 (void *b, uint16_t x) {
	uint8_t* buffer = (uint8_t*) b;
	buffer[0] = x & 0xff;
	buffer[1] = x >> 8;
}
void static inline put_lsb32 (void *b, uint32_t x) {
	uint8_t* buffer = (uint8_t*) b;
	buffer[0] = x & 0xff;
	buffer[1] = x >>  8;
	buffer[2] = x >> 16;
	buffer[3] = x >> 24;
}

VGMCapture::VGMCapture(FILE *theHandle) {
	handle = theHandle;
	totalSamples = tickCount = 0;
	YM2413_used = APU1_used = APU2_used = false;
}

VGMCapture::~VGMCapture() {
	try {	logTimeDifference();
		buffer.push_back(CMD_END_OF_SOUND_DATA);
	
		/* Build the .VGM header */
		size_t headerSize = 0x100;
		size_t vgmSize = headerSize + buffer.size();
		memset(&header, 0, sizeof(header));
		memcpy(header.id, "Vgm ", 4);
		put_lsb32(&header.version, 0x161);
		put_lsb32(&header.samplesInFile, totalSamples);
		put_lsb32(&header.rofsEOF, vgmSize -offsetof(VGMHeader, rofsEOF));
		put_lsb32(&header.rofsData, headerSize -offsetof(VGMHeader, rofsData));
		if (YM2413_used) {
			put_lsb32(&header.clockYM2413, 3579545);
		}
		if (APU1_used) {
			put_lsb32(&header.clockNESapu, 1789772);
		}
		if (APU2_used) {
			put_lsb32(&header.clockNESapu, 1789772 | 0x40000000);
		}
		fseek(handle, 0, SEEK_SET);
		fwrite(&header, 1, headerSize, handle);
		fwrite(&buffer[0], 1, buffer.size(), handle);
		fclose(handle);
	} catch(...) { }
}

void VGMCapture::logTimeDifference(void) {
	float ticksPassed = (float) tickCount * 11.0 / 19875.0;
	float samplesPassedFloat = ticksPassed * 44100.0 / 1000.0 +samplesPassedFraction;
	uint32_t samplesPassed = samplesPassedFloat;
	samplesPassedFraction = samplesPassedFloat -samplesPassed;
	totalSamples += samplesPassed;
	while (samplesPassed) {
		uint16_t interval = (samplesPassed > 65535)? 65535: samplesPassed;
		if (interval <= 16) { /* Intervals from 1-16 can be expressed in one byte as one 7x command */
			buffer.push_back(CMD_WAIT_SHORT + interval -1);
		} else
		if (interval <= 32) { /* Intervals from 17-32 can be expressed in two bytes as two 7x commands (16 plus x) */
			buffer.push_back(CMD_WAIT_SHORT + 16 -1);
			buffer.push_back(CMD_WAIT_SHORT + interval -16 -1);
		} else
		switch(interval) {
			case 735:  buffer.push_back(CMD_WAIT_735_SAMPLES); break;
			case 882:  buffer.push_back(CMD_WAIT_882_SAMPLES); break;
			case 1470: buffer.push_back(CMD_WAIT_735_SAMPLES); buffer.push_back(CMD_WAIT_735_SAMPLES); break;
			case 1617: buffer.push_back(CMD_WAIT_735_SAMPLES); buffer.push_back(CMD_WAIT_882_SAMPLES); break;
			case 1764: buffer.push_back(CMD_WAIT_882_SAMPLES); buffer.push_back(CMD_WAIT_882_SAMPLES); break;
			default:   buffer.push_back(CMD_WAIT_N_SAMPLES);
				   buffer.push_back(interval & 0xFF);
				   buffer.push_back(interval >> 8);
		}
		samplesPassed -= interval;
	}
	tickCount =0;
}


void VGMCapture::ioWrite_YM2413(uint8_t index, uint8_t value) {
	logTimeDifference();
	YM2413_used = true;
	buffer.push_back(CMD_YM2413_WRITE);
	buffer.push_back(index);
	buffer.push_back(value);
}

void VGMCapture::ioWrite_APU1(uint8_t index, uint8_t value) {
	logTimeDifference();
	APU1_used = true;
	buffer.push_back(CMD_NESAPU_WRITE);
	buffer.push_back(index);
	buffer.push_back(value);
}

void VGMCapture::ioWrite_APU2(uint8_t index, uint8_t value) {
	logTimeDifference();
	APU2_used = true;
	buffer.push_back(CMD_NESAPU_WRITE);
	buffer.push_back(index | 0x80);
	buffer.push_back(value);
}

void VGMCapture::nextTick() {
	tickCount++;
}