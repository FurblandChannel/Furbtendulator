#ifndef INCLUDED_FDSFILE_FWNES_HPP
#define INCLUDED_FDSFILE_FWNES_HPP 1
#include "FDSFile.hpp"

struct FDSFile_fwNES: public FDSFile {
	std::string                                type() const { return "fwNES"; };
	bool                                       allowMultipleSides() const { return true; }
	static bool                                detect(const std::vector<uint8_t>&);
	std::wstring                               getNameForSide (const std::wstring&, const int) const;
	std::vector<uint8_t>                       outputSide(int) const;
	virtual std::vector<std::vector<uint8_t> > outputSides() const override;
	
	FDSFile_fwNES(): FDSFile() { };
	FDSFile_fwNES(const std::vector<uint8_t>&, FDSProtection&);
	FDSFile_fwNES(const FDSFile& other): FDSFile(other) { };
};

struct FDSFile_fwNES_Headerless: public FDSFile_fwNES {
	std::string                        type() const { return "fwNES Headerless"; };
	static bool                        detect(const std::vector<uint8_t>&);
	std::vector<std::vector<uint8_t> > outputSides() const override;
	
	using FDSFile_fwNES::FDSFile_fwNES;
};

#endif