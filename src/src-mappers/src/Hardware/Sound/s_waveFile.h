#pragma once
#include <windows.h>
#include <vector>

class WaveFile {
	int                   bits;
	int                   rate;
	int                   fetchRate;
	int                   fetchCount;
	int                   channels;
	uint32_t              position;
	bool                  finished;
	std::vector <int16_t> data;
public:
		WaveFile      (TCHAR *);
		WaveFile      (const int16_t*, size_t, int);
	void	setFetchRate  (int);
	bool	isFinished    ();
	void	restart       ();
	int	getNextSample ();
};
typedef std::vector<WaveFile> WaveFiles;

void loadWaveFiles (std::vector<WaveFile>&, TCHAR *, int);