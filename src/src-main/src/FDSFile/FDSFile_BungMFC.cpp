#include "FDSFile_BungMFC.hpp"

bool FDSFile_BungMFC::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=3 &&
	       diskData.at(0) ==0x00 &&
	       diskData.at(1) ==0x00 &&
	       diskData.at(2) >=0x80
	;
}

std::wstring FDSFile_BungMFC::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base + (wchar_t) '.';
	result.append(1, 'A' +side);
	return result;
}

FDSFile_BungMFC::FDSFile_BungMFC (const std::vector<uint8_t>& sideData, FDSProtection& protection) {
	std::vector<FDSBlock> side;
	unsigned int filesLeft =sideData.at(2) &0x7F;
	uint32_t pos =3;

	FDSBlock diskHeader(0x01, sideData, pos, 55);
	protection.notify(diskHeader);
	side.push_back(diskHeader);
	pos +=2;

	FDSBlock fileCount(0x02, sideData, pos, 1);
	protection.notify(fileCount);
	side.push_back(fileCount);
	pos +=2;

	while(filesLeft--) {
		FDSBlock fileHeader(0x03, sideData, pos, 15);
		protection.notify(fileHeader);
		pos +=2;

		FDSBlock fileData(0x04, sideData, pos, protection.getFileSize());
		protection.notify(fileData);
		pos +=2;

		side.push_back(fileHeader);
		side.push_back(fileData);
	}
	// No hidden files here, since the Bung header contains the actual number of files.
	
	if (protection.epiloguePresent()) {
		FDSBlock epilogueData(protection.epilogueType(), sideData, pos, protection.epilogueSize());
		protection.notify(epilogueData);
		side.push_back(epilogueData);
		pos +=2;		
	}
	data.push_back(side);
}

std::vector<uint8_t> FDSFile_BungMFC::outputSide(int side) const {
	std::vector<uint8_t> result(3, 0);
	unsigned int numberOfFiles =0;
	for (auto& block: data.at(side)) {
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());
		result.push_back(0x00);
		result.push_back(0x00);
		if (block.type ==0x03) numberOfFiles++;
	}
	result[2] =numberOfFiles |0x80;
	return result;
}