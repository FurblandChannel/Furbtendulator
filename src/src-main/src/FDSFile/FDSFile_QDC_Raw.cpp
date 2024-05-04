#include "FDSFile_QDC_Raw.hpp"
#include <math.h>

bool FDSFile_QDC_Raw::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=8 &&
  	    diskData[0] ==0x00 &&
	    diskData[1] ==0x00 &&
	    diskData[2] ==0x00 &&
	    diskData[3] ==0x00 &&
	    diskData[diskData.size() -1] ==0xFF &&
	    diskData[diskData.size() -2] ==0x00 &&
	    diskData[diskData.size() -3] ==0x00 &&
	    diskData[diskData.size() -4] ==0x00;
}

std::wstring FDSFile_QDC_Raw::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base;
	if (data.size() >1) {
		result +='-';
		result +=('A' +side);
	}
	result +=L".raw";
	return result;
}

FDSFile_QDC_Raw::FDSFile_QDC_Raw (const std::vector<uint8_t>& sideData, FDSProtection& protection) {
	std::vector<bool> mfm =flux2MFM(sideData, 2816.0F/225.0F);

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

std::vector<uint8_t> FDSFile_QDC_Raw::outputSide(int side) const {
	std::vector<uint8_t> binaryData =FDSBlocks2Binary(data.at(side), 32768);
	std::vector<bool>    mfmData    =binary2MFM      (binaryData);
	std::vector<uint8_t> fluxData   =mfm2Flux        (mfmData, 2816, 225);
	
	std::vector<uint8_t> result;
	result.push_back(0x00);
	result.push_back(0x00);
	result.push_back(0x00);
	result.push_back(0x00);
	result.insert(result.end(), fluxData.begin(), fluxData.end());
	result.push_back(0x00);
	result.push_back(0x00);
	result.push_back(0x00);
	result.push_back(0xFF);
	return result;
}