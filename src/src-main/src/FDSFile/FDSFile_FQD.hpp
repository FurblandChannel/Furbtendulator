#ifndef INCLUDED_FDSFILE_FQD_HPP
#define INCLUDED_FDSFILE_FQD_HPP 1
#include "FDSFile.hpp"

struct FDSFile_FQD: public FDSFile {
	int                                targetDevice;
	int                                targetConfig;
	int                                expDevice;
	std::string                        type() const { return "Fancy QuickDisk"; };
	bool                               allowMultipleSides() const { return true; }
	static bool                        detect(const std::vector<uint8_t>&);
	std::wstring                       getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>               outputSide(int) const;
	std::vector<std::vector<uint8_t> > outputSides() const override;

	FDSFile_FQD(): FDSFile() { targetDevice =targetConfig =expDevice =0; };
	FDSFile_FQD(int a, int b, int c): FDSFile() { targetDevice =a; targetConfig =b; expDevice =c; };
	FDSFile_FQD(const std::vector<uint8_t>&);
	FDSFile_FQD(const FDSFile& other): FDSFile(other) { targetDevice =targetConfig =expDevice =0; };
};
#endif