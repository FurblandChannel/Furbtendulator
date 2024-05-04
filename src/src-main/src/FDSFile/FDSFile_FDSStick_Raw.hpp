#ifndef INCLUDED_FDSFILE_FDSSTICK_RAW_HPP
#define INCLUDED_FDSFILE_FDSSTICK_RAW_HPP 1
#include "FDSFile.hpp"

struct FDSFile_FDSStick_Raw: public FDSFile {
	std::string                       type() const { return "FDSStick 8-Bit Flux Reversal Data"; };
	bool                              allowMultipleSides() const { return false; }
	static bool                       detect(const std::vector<uint8_t>&);
	std::wstring                      getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>              outputSide(int) const;

	FDSFile_FDSStick_Raw(): FDSFile() { };
	FDSFile_FDSStick_Raw(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_FDSStick_Raw(const FDSFile& other): FDSFile(other) { };
};
#endif