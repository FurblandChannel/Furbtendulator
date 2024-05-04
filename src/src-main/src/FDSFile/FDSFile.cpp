#include "FDSFile.hpp"
#include "FDSFile_BungMFC.hpp"
#include "FDSFile_FQD.hpp"
#include "FDSFile_FDSStick_Binary.hpp"
#include "FDSFile_FDSStick_Raw.hpp"
#include "FDSFile_FDSStick_Raw03.hpp"
#include "FDSFile_fwNES.hpp"
#include "FDSFile_NintendoQD.hpp"
#include "FDSFile_QDC_Raw.hpp"
#include "FDSFile_Venus.hpp"
#include "FDSFile_FFE.hpp"

FDSFile::FDSFile(const FDSFile& other) {
	data =other.data;
}

void FDSFile::addSides(const FDSFile& other) {
	for (auto& s: other.data) data.push_back(s);
}

std::unique_ptr<FDSFile> FDSFile::autodetect(const std::vector<uint8_t>& data, FDSProtection& protection) {
	if (FDSFile_fwNES::detect(data))
		return std::make_unique<FDSFile_fwNES>(data, protection);
	else
	if (FDSFile_fwNES_Headerless::detect(data))
		return std::make_unique<FDSFile_fwNES_Headerless>(data, protection);
	else
	if (FDSFile_FQD::detect(data))
		return std::make_unique<FDSFile_FQD>(data);
	else
	if (FDSFile_NintendoQD::detect(data))
		return std::make_unique<FDSFile_NintendoQD>(data, protection);
	else
	if (FDSFile_BungMFC::detect(data))
		return std::make_unique<FDSFile_BungMFC>(data, protection);
	else
	if (FDSFile_Venus::detect(data))
		return std::make_unique<FDSFile_Venus>(data, protection);
	else
	if (FDSFile_FFE::detect(data))
		return std::make_unique<FDSFile_FFE>(data, protection);
	else
	if (FDSFile_FDSStick_Binary::detect(data))
		return std::make_unique<FDSFile_FDSStick_Binary>(data, protection);
	else
	if (FDSFile_FDSStick_Raw::detect(data))
		return std::make_unique<FDSFile_FDSStick_Raw>(data, protection);
	else
	if (FDSFile_FDSStick_Raw03::detect(data))
		return std::make_unique<FDSFile_FDSStick_Raw03>(data, protection);
	else
	if (FDSFile_QDC_Raw::detect(data))
		return std::make_unique<FDSFile_QDC_Raw>(data, protection);
	else
		throw std::runtime_error("Unrecognized format");
}

std::vector<std::vector<uint8_t> > FDSFile::outputSides() const {
	// Default implementation: One file per side
	std::vector<std::vector<uint8_t> > result;
	for (unsigned int side =0; side <data.size(); side++) {
		result.push_back(outputSide(side));
	}	
	return result;	
}

std::vector<bool> flux2MFM (const std::vector<uint8_t>& fluxData, float cellSize) {
	std::vector<bool> result;
	
	float pulse =0.0;
	float lastCell =cellSize;
	for (size_t i =0; i <fluxData.size(); i++) {
		pulse +=fluxData.at(i) /lastCell;
		if (fluxData.at(i) ==255) continue; // byte 255 means overflow without flux reversal here
		while (pulse >=1.5) {
			pulse -=1.0;
			result.push_back(false);
		}
		result.push_back(true);
		pulse -=1.0;
		if (pulse <0.0)
			lastCell *= 0.9995F;
		else if (pulse >0.0)
			lastCell *= 1.0005F;
		// Allow up to 10 percent deviation
		if (lastCell <cellSize*0.9) lastCell =0.9*cellSize;
		if (lastCell >cellSize*1.1) lastCell =1.1*cellSize;
		pulse *=0.5;
	}
	return result;
}

std::vector<uint8_t> FDSBlocks2Binary (const std::vector<FDSBlock>& blockData, int leadInBits) {
	std::vector<uint8_t> result;
	
	for (int i =0; i <leadInBits /8; i++) result.push_back(0x00);	
	for (auto& block: blockData) {
		result.push_back(0x80); // Gap end mark
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());
		result.push_back(block.crc &0xFF);
		result.push_back(block.crc >>8);
		for (int i =0; i <125; i++) result.push_back(0x00); // Between-block gap, nominally 125 bytes or 2,000 raw MFM bits
	}
	return result;
}

std::vector<bool> binary2MFM (const std::vector<uint8_t>& binaryData) {
	std::vector<bool> result;
	
	bool lastBit =true;
	for (auto c: binaryData) {
		for (int i =0; i <8; i++) {
			bool bit =!!(c &0x01);
			result.push_back(bit? false: !lastBit); // MFM clock bit
			result.push_back(bit);                  // MFM data bit
			c >>=1;
			lastBit =bit;
		}
	}
	return result;
}

std::vector<uint8_t> mfm2Flux (const std::vector<bool>& mfmData, int cellSizeNumerator, int cellSizeDenominator) {
	std::vector<uint8_t> result;
	int bits =0;
	int rest =0;
	for (auto c: mfmData) {
		bits++;
		if (c) {
			int duration =(bits *cellSizeNumerator +rest) /cellSizeDenominator;
		            rest     =(bits *cellSizeNumerator +rest) %cellSizeDenominator;
			while (duration >=255) {
				result.push_back(255);
				duration -=255;
			}
			result.push_back(duration);
			bits =0;
		}
	}
	return result;
}
