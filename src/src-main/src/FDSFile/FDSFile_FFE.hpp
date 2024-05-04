#ifndef INCLUDED_FDSFILE_FFE_HPP
#define INCLUDED_FDSFILE_FFE_HPP 1
#include "FDSFile.hpp"

struct FDSBlockFFE: public FDSBlock {
	FDSBlockFFE (const uint8_t, const std::vector<uint8_t>&, uint32_t& pos, size_t); // size_t without block type byte or CRC byte
};

struct FDSFile_FFE: public FDSFile {
	int                                catalogue;
	std::string                        type() const { return "Front Fareast"; };
	bool                               allowMultipleSides() const { return true; }
	static bool                        detect(const std::vector<uint8_t>&);
	std::wstring                       getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>               outputSide(int) const;
	std::vector<std::vector<uint8_t> > outputSides() const override;

	FDSFile_FFE(): FDSFile() { catalogue =0; };
	FDSFile_FFE(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_FFE(int _catalogue): FDSFile() { catalogue =_catalogue; };
	FDSFile_FFE(const FDSFile& other): FDSFile(other) { };
};
#endif