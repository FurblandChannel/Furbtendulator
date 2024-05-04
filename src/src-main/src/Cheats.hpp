#pragma once

namespace Cheats {
struct CheatLocation {
	uint8_t bitmask;
	uint32_t address;
	std::vector<uint8_t> compare;
	std::vector<uint8_t> replace;
	void clear();
	CheatLocation();
};
struct Cheat {
	bool enabled;
	std::wstring name;
	std::vector<CheatLocation> locations;
	void clear();
	Cheat() { clear(); };
};

struct Game {
	std::wstring fdsID;
	std::vector<uint32_t> crcs;
	std::vector<Cheat> cheats;	
	void clear();
	bool hasCRC (uint32_t wantedCRC);
	Game() { clear(); };
};

std::vector<Game> parseCheatInfo(const std::vector<wchar_t>&);
void cheatsWindow (const HWND hWnd);
void findCheats ();
void findFDSCheats (std::wstring&);
uint8_t readCPUMemory (unsigned int, uint8_t);
}
