namespace FDSPatch {
struct Item {
	uint32_t address;
	std::vector<uint8_t> compare;
	std::vector<uint8_t> replace;
	void clear();
};

struct Game {
	std::wstring name;
	bool ignoreNMI;
	std::vector<Item> items;	
	void clear();
};

std::vector<Game> parsePAT(const std::vector<wchar_t>& text);

} // namespace FDSPatch