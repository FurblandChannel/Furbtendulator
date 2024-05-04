#include <string>
#include <vector>

namespace PlugThruDevice {
namespace StudyBox {
struct StudyBoxFile {
	FILE* handle;
	struct Page {
		uint32_t leadIn;
		uint32_t start;
		std::vector<uint8_t> data;
		Page(uint32_t, uint32_t, uint32_t);
	};
	std::vector<Page> pages;
	uint32_t rate;
	std::vector<int16_t> audio;
	
	std::string readFourCC() const;
	uint32_t readLSB32() const;
	uint32_t readLSB16() const;
public:	
	StudyBoxFile(FILE*);
};
extern  StudyBoxFile* cassette;
int	MAPINT	mapperSnd (int cycles);
} // namespace StudyBox
} // namespace PlugThruDevice
