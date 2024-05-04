#include "..\..\interface.h"
#include "s_waveFile.h"
#include <tchar.h>
#include <stdio.h>

WaveFile::WaveFile (TCHAR *name) {
	bits =16;
	rate =44100;
	fetchRate =1789773;
	fetchCount =0;
	channels =1;
	position =0;
	data.clear();
	
	FILE *handle =_tfopen(name, _T("rb"));
	if (handle) {
		uint8_t header[0x2C];
		if (fread(header, 1, 0x2C, handle) ==0x2C &&
		    !memcmp(header +0x00, "RIFF", 4) &&
		    !memcmp(header +0x08, "WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00", 16) &&
		    !memcmp(header +0x20,"\x02\x00\x10\x00", 4)) {
			rate =header[0x18] | header[0x19] <<8 | header[0x1A] <<16 | header[0x1B] <<24;
			data.resize((header[0x28] | header[0x29] <<8 | header[0x2A] <<16 | header[0x2B] <<24) /2);
			fread(&data[0], 2, data.size(), handle);
		}
		fclose(handle);
	}
	finished =false;
}

WaveFile::WaveFile(const int16_t* _data, size_t _size, int _rate) {
	rate =_rate;
	bits =16;
	channels =1;
	fetchRate =1789773;
	fetchCount =0;
	channels =1;
	position =0;
	data.clear();
	for (unsigned int i =0; i <_size; i++) data.push_back(_data[i]);
	finished =false;
}

void WaveFile::setFetchRate (int _rate) {
	fetchRate =_rate;
}

bool WaveFile::isFinished () {
	return finished;
}

void WaveFile::restart () {
	position =0;
	fetchCount =0;
	finished =false;
}

int WaveFile::getNextSample () {
	int result =0;
	
	finished =position >=data.size();
	if (!finished) {
		result =data[position];
		fetchCount +=rate;
		while (fetchCount >=fetchRate) {
			position++;
			fetchCount -=fetchRate;
		}
		finished =position >=data.size();
	}
	return result;
}

void loadWaveFiles (std::vector<WaveFile>& waveFiles, TCHAR *dirName, int num) {
	waveFiles.clear();
	TCHAR buf[256];
	TCHAR buf2[256];
	int i =GetModuleFileName(NULL, buf, 256);
	if (i) {
		while (i > 0)
			if (buf[--i] =='\\')
				break;
		buf[i++] =0;
	}
	for (int sample =0; sample <num; sample++) {
		_stprintf(buf2, _T("%s\\samples\\%s\\%02d.wav"), buf, dirName, sample);
		WaveFile wave(buf2);
		waveFiles.push_back(wave);
	}
}