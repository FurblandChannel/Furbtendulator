namespace DIP {
struct Choice {
	std::wstring name;
	uint32_t value;
	void clear();
};

struct Setting {
	std::wstring name;
	uint32_t mask;
	uint32_t defaultValue;
	std::vector<Choice> choices;
	void clear();
};

struct Game {
	std::wstring name;
	std::vector<uint32_t> crcs;
	std::vector<Setting> settings;
	void clear() { name =L""; crcs.clear(); settings.clear(); }
	bool hasCRC (uint32_t wantedCRC);
	uint32_t getDefaultValue();
};

std::vector<Game> parseDIP(const std::vector<wchar_t>& text);
bool windowDIP(const HWND hWnd, const Game& game, uint32_t& value);

} // namespace DIP