#include "FDSFile_FDSStick_Raw03.hpp"
#include <math.h>

bool FDSFile_FDSStick_Raw03::detect(const std::vector<uint8_t>& diskData) {
	// Size between 32k and 200k, max. 25% out-of-range samples
	unsigned int numberOfOutOfRangeSamples =0;
	for (auto& c: diskData) {
		if ((c >>6 &3) ==3) numberOfOutOfRangeSamples++;
		if ((c >>4 &3) ==3) numberOfOutOfRangeSamples++;
		if ((c >>2 &3) ==3) numberOfOutOfRangeSamples++;
		if ((c >>0 &3) ==3) numberOfOutOfRangeSamples++;
	}
	return diskData.size() >150000/4 && diskData.size() <750000/4 && (numberOfOutOfRangeSamples *100/(diskData.size() *4)) <25;
}

std::wstring FDSFile_FDSStick_Raw03::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base;
	if (data.size() >1) {
		result +='-';
		result +=('A' +side);
	}
	result +=L".raw";
	return result;
}

FDSFile_FDSStick_Raw03::FDSFile_FDSStick_Raw03 (const std::vector<uint8_t>& sideData, FDSProtection& protection) {
	// Inflate Raw03 to Raw data
	std::vector<uint8_t> rawData;
	for (auto& c: sideData) {		
		rawData.push_back(((c >>0 &3) +2) *1408.0F/45.0F);
		rawData.push_back(((c >>2 &3) +2) *1408.0F/45.0F);
		rawData.push_back(((c >>4 &3) +2) *1408.0F/45.0F);
		rawData.push_back(((c >>6 &3) +2) *1408.0F/45.0F);
	}
	std::vector<bool> mfm =flux2MFM(rawData, 1408.0F/45.0F);

	// Decode MFM bitcells
	std::vector<FDSBlock> side;
	uint32_t pos =0;
	
	FDSBlockMFM diskHeader(0x01, mfm, pos, 55);
	protection.notify(diskHeader);
	side.push_back(diskHeader);

	FDSBlockMFM fileCount(0x02, mfm, pos, 1);
	protection.notify(fileCount);
	side.push_back(fileCount);

	unsigned int filesLeft =fileCount.data.at(0);
	while(filesLeft--) {
		FDSBlockMFM fileHeader(0x03, mfm, pos, 15);
		protection.notify(fileHeader);

		FDSBlockMFM fileData(0x04, mfm, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}

	uint32_t lastPos;
	try {   // Process any hidden files
	while(1) {
		lastPos =pos;
		FDSBlockMFM fileHeader(0x03, mfm, pos, 15);
		protection.notify(fileHeader);

		lastPos =pos;
		FDSBlockMFM fileData(0x04, mfm, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}
	} catch(...) { pos =lastPos; }

	// Process post-file data
	if (protection.epiloguePresent()) {
		FDSBlockMFM epilogueData(protection.epilogueType(), mfm, pos, protection.epilogueSize());
		protection.notify(epilogueData);
		side.push_back(epilogueData);
	}
	
	data.push_back(side);
}

std::vector<uint8_t> FDSFile_FDSStick_Raw03::outputSide(int side) const {
	std::vector<uint8_t> binaryData =FDSBlocks2Binary(data.at(side), 32768);
	std::vector<bool>    mfmData    =binary2MFM      (binaryData);
	std::vector<uint8_t> fluxData   =mfm2Flux        (mfmData, 1408, 45);
	
	std::vector<uint8_t> raw03Data;
	uint8_t outByte =0;
	for (unsigned int i =0; i <fluxData.size(); i++) {
		uint8_t inByte =fluxData[i];
		uint8_t outBits;
		     if (inByte <0x30) outBits =3;
		else if (inByte <0x50) outBits =0;
		else if (inByte <0x70) outBits =1;
		else if (inByte <0xA0) outBits =2;
		else                   outBits =3;
		outByte =outByte >>2 |outBits <<6;
		if ((i &3) ==3) raw03Data.push_back(outByte);
	}
	return raw03Data;
}