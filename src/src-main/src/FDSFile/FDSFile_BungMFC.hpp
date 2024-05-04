#ifndef INCLUDED_FDSFILE_BUNGMFC_HPP
#define INCLUDED_FDSFILE_BUNGMFC_HPP 1
#include "FDSFile.hpp"

struct FDSFile_BungMFC: public FDSFile {
	std::string                       type() const { return "Bung MFC"; };
	bool                              allowMultipleSides() const { return false; }
	static bool                       detect(const std::vector<uint8_t>&);
	std::wstring                      getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>              outputSide(int) const;

	FDSFile_BungMFC(): FDSFile() { };
	FDSFile_BungMFC(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_BungMFC(const FDSFile& other): FDSFile(other) { };
};
#endif