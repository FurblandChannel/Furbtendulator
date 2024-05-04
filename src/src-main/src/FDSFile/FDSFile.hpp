#ifndef INCLUDED_FDSFILE_HPP
#define INCLUDED_FDSFILE_HPP 1
#include "../stdafx.h"
#include "../Nintendulator.h"
#include "../MapperInterface.h"

#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <string>
#include <vector>
#include "to_string.hpp"
#include "FDSBlock.hpp"
#include "FDSProtection.hpp"

struct FDSFile {
	// Several FDS blocks in each of several sides
	std::vector<std::vector<FDSBlock> >        data;

	// Get the format name as text, e.g. "fwNES"
	virtual std::string                        type() const =0;
	
	// Return whether the format allows multiple sides per file
	virtual bool                               allowMultipleSides() const =0;

	// Get FDSFile object of correct derived class based on auto-detection from given input
	static std::unique_ptr<FDSFile>            autodetect(const std::vector<uint8_t>&, FDSProtection&);

	// Get the full filename for a given base name (without extension and without dot) and a given side number
	virtual std::wstring                       getNameForSide (const std::wstring&, const int) const =0;

	// Output a single side's data
	virtual std::vector<uint8_t>               outputSide(int) const =0;

	// Output a vector of several sides each containing a single side's data.
	// The default implementation just creates one outer vector element from calling output(int) for each side, which is appropriate for single-side-per-file formats.
	virtual std::vector<std::vector<uint8_t> > outputSides() const;

	// Add sides from another FDSFile to this FDSFile
	void                                       addSides(const FDSFile&);

	FDSFile() {}

	// Create FDSFile from another FDSFile, usually of a different type
	FDSFile(const FDSFile&);
};

// Helper functions
std::vector<bool>    flux2MFM         (const std::vector<uint8_t>&, float);
std::vector<uint8_t> FDSBlocks2Binary (const std::vector<FDSBlock>&, int);
std::vector<bool>    binary2MFM       (const std::vector<uint8_t>&);
std::vector<uint8_t> mfm2Flux         (const std::vector<bool>&, int, int);

#endif