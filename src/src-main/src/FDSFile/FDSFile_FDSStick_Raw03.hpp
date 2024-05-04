#ifndef INCLUDED_FDSFILE_FDSSTICK_RAW03_HPP
#define INCLUDED_FDSFILE_FDSSTICK_RAW03_HPP 1
#include "FDSFile.hpp"
#include "FDSFile_FDSStick_Raw.hpp"

struct FDSFile_FDSStick_Raw03: public FDSFile {
	std::string                       type() const { return "FDSStick 2-Bit Flux Reversal Data"; };
	bool                              allowMultipleSides() const { return false; }
	static bool                       detect(const std::vector<uint8_t>&);
	std::wstring                      getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>              outputSide(int) const;

	FDSFile_FDSStick_Raw03(): FDSFile() { };
	FDSFile_FDSStick_Raw03(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_FDSStick_Raw03(const FDSFile& other): FDSFile(other) { };
};
#endif