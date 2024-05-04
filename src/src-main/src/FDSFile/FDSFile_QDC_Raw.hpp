#ifndef INCLUDED_FDSFILE_QDC_RAW_HPP
#define INCLUDED_FDSFILE_QDC_RAW_HPP 1
#include "FDSFile.hpp"
#include "FDSFile_FDSStick_Raw.hpp"

struct FDSFile_QDC_Raw: public FDSFile {
	std::string                       type() const { return "QDC 8-Bit Flux Reversal Data"; };
	bool                              allowMultipleSides() const { return false; }
	static bool                       detect(const std::vector<uint8_t>&);
	std::wstring                      getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>              outputSide(int) const;

	FDSFile_QDC_Raw(): FDSFile() { };
	FDSFile_QDC_Raw(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_QDC_Raw(const FDSFile& other): FDSFile(other) { };
};
#endif