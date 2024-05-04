#ifndef INCLUDED_FDSFILE_FDSSTICK_BINARY_HPP
#define INCLUDED_FDSFILE_FDSSTICK_BINARY_HPP 1
#include "FDSFile.hpp"

struct FDSFile_FDSStick_Binary: public FDSFile {
	std::string                        type() const { return "FDSStick Binary Data"; };
	bool                               allowMultipleSides() const { return true; }
	static bool                        detect(const std::vector<uint8_t>&);
	std::wstring                       getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>               outputSide(int) const;

	FDSFile_FDSStick_Binary(): FDSFile() { };
	FDSFile_FDSStick_Binary(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_FDSStick_Binary(const FDSFile& other): FDSFile(other) { };
};
#endif