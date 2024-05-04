#ifndef INCLUDED_FDSFILE_VENUS_HPP
#define INCLUDED_FDSFILE_VENUS_HPP 1
#include "FDSFile.hpp"

struct FDSBlockWithSum: public FDSBlock {
	FDSBlockWithSum (const uint8_t, const std::vector<uint8_t>&, uint32_t& pos, size_t); // size_t without block type byte or sum word
};

struct FDSFile_Venus: public FDSFile {
	std::string                       type() const { return "Venus"; };
	bool                              allowMultipleSides() const { return false; }
	static bool                       detect(const std::vector<uint8_t>&);
	std::wstring                      getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>              outputSide(int) const;

	FDSFile_Venus(): FDSFile() { };
	FDSFile_Venus(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_Venus(const FDSFile& other): FDSFile(other) { };
};
#endif