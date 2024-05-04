#include "FDSFile_FDSStick_Binary.hpp"
#include <algorithm>

bool FDSFile_FDSStick_Binary::detect(const std::vector<uint8_t>& diskData) {
	const static uint8_t compare[] ="\x00\x00\x00\x00\x80\x01*NINTENDO-HVC*";
	return std::search(diskData.begin(), diskData.end(), compare, compare +sizeof(compare) -1) !=diskData.end();
}

std::wstring FDSFile_FDSStick_Binary::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base;
	if (data.size() >1) {
		result +='-';
		result +=('A' +side);
	}
	result +=L".bin";
	return result;
}

FDSFile_FDSStick_Binary::FDSFile_FDSStick_Binary (const std::vector<uint8_t>& diskData, FDSProtection& protection) {
	std::vector<FDSBlock> side;
	uint32_t pos =0;

	while (diskData.at(pos++) !=0x80 || diskData.at(pos +0) !=0x01 || diskData.at(pos +1) !='*');
	FDSBlockWithCRC diskHeader(0x01, diskData, pos, 55);
	protection.notify(diskHeader);
	side.push_back(diskHeader);

	pos +=18; while (diskData.at(pos++) !=0x80);
	FDSBlockWithCRC fileCount(0x02, diskData, pos, 1);
	protection.notify(fileCount);
	side.push_back(fileCount);

	unsigned int filesLeft =fileCount.data.at(0);
	while(filesLeft--) {
		pos +=18; while (diskData.at(pos++) !=0x80);
		FDSBlockWithCRC fileHeader(0x03, diskData, pos, 15);
		protection.notify(fileHeader);

		pos +=18; while (diskData.at(pos++) !=0x80);
		FDSBlockWithCRC fileData(0x04, diskData, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}

	uint32_t lastPos;
	try {	// Process any hidden files
	while(1) {
		lastPos =pos;
		pos +=18; while (diskData.at(pos++) !=0x80);
		FDSBlockWithCRC fileHeader(0x03, diskData, pos, 15);
		protection.notify(fileHeader);

		lastPos =pos;
		pos +=18; while (diskData.at(pos++) !=0x80);
		FDSBlockWithCRC fileData(0x04, diskData, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}
	} catch(...) { pos =lastPos; }

	// Process post-file data
	if (protection.epiloguePresent()) {
		pos +=18; while (diskData.at(pos++) !=0x80);
		FDSBlockWithCRC epilogueData(protection.epilogueType(), diskData, pos, protection.epilogueSize());
		protection.notify(epilogueData);
		side.push_back(epilogueData);
	}
	
	data.push_back(side);
}

std::vector<uint8_t> FDSFile_FDSStick_Binary::outputSide(int side) const {
	return FDSBlocks2Binary(data.at(side), 40000);
}