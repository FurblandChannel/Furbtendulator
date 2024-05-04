#ifndef INCLUDED_FDSFILE_NINTENDOQD_HPP
#define INCLUDED_FDSFILE_NINTENDOQD_HPP 1
#include "FDSFile.hpp"

struct FDSFile_NintendoQD: public FDSFile {
	std::string                        type() const { return "Nintendo QD"; };
	bool                               allowMultipleSides() const { return true; }
	static bool                        detect(const std::vector<uint8_t>&);
	std::wstring                       getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>               outputSide(int) const;
	std::vector<std::vector<uint8_t> > outputSides() const override;

	FDSFile_NintendoQD(): FDSFile() { };
	FDSFile_NintendoQD(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_NintendoQD(const FDSFile& other): FDSFile(other) { };
};
#endif