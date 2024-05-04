#ifndef INCLUDED_FDSBLOCK_HPP
#define INCLUDED_FDSBLOCK_HPP 1

class CRC16 {
	uint32_t crc;
public:
	CRC16();
	uint16_t get() const;
	void     add(uint8_t);
};

struct FDSBlock {
	uint8_t type;
	std::vector<uint8_t> data;
	uint16_t crc;
	uint16_t sum;

	FDSBlock ();
	FDSBlock (const uint8_t, const std::vector<uint8_t>&, uint32_t& pos, size_t); // size_t without block type byte
};

struct FDSBlockWithCRC: public FDSBlock {
	FDSBlockWithCRC (const uint8_t, const std::vector<uint8_t>&, uint32_t& pos, size_t); // size_t without block type byte or CRC word
};

struct FDSBlockMFM: public FDSBlock {
	        FDSBlockMFM (const uint8_t, const std::vector<bool>&, uint32_t& pos, size_t);
 static uint8_t getByte     (const std::vector<bool>&, uint32_t);   // Decode MFM word to byte at position, do not advance position
 static uint8_t getNextByte (const std::vector<bool>&, uint32_t&);  // Decode MFM word to byte at position and advance position
};

#endif