#include "FDSFile_Venus.hpp"

FDSBlockWithSum::FDSBlockWithSum(const uint8_t wantedType, const std::vector<uint8_t>& input, uint32_t& pos, size_t size):
	FDSBlock(wantedType, input, pos, size) {
	uint16_t foundSum =input.at(pos +0) | input.at(pos +1) <<8;
	pos +=2;
	if (foundSum !=sum) EI.DbgOut(L"Wrong sum (%04X instead of %04X) at position $%X\n", foundSum, sum, pos);
	sum =foundSum;
}

bool FDSFile_Venus::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=4 &&
	       diskData.at(0) ==(((diskData.size() -3) >> 0) &0xFF) &&
	       diskData.at(1) ==(((diskData.size() -3) >> 8) &0xFF) &&
	       diskData.at(2) ==(((diskData.size() -3) >>16) &0xFF) &&
	       diskData.at(3) ==0x01;
}

std::wstring FDSFile_Venus::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base +(wchar_t) '.';
	result.append(1, 'A' +side);
	return result;
}

FDSFile_Venus::FDSFile_Venus (const std::vector<uint8_t>& sideData, FDSProtection& protection) {
	std::vector<FDSBlock> side;
	uint32_t pos =3;

	FDSBlockWithSum diskHeader(0x01, sideData, pos, 55);
	protection.notify(diskHeader);
	side.push_back(diskHeader);

	FDSBlockWithSum fileCount(0x02, sideData, pos, 1);
	protection.notify(fileCount);
	side.push_back(fileCount);

	unsigned int filesLeft =fileCount.data.at(0);
	while(filesLeft--) {
		FDSBlockWithSum fileHeader(0x03, sideData, pos, 15);
		protection.notify(fileHeader);

		FDSBlockWithSum fileData(0x04, sideData, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}

	uint32_t lastPos;
	try {   // Process any hidden files
	while(1) {
		lastPos =pos;
		FDSBlockWithSum fileHeader(0x03, sideData, pos, 15);
		protection.notify(fileHeader);

		lastPos =pos;
		FDSBlockWithSum fileData(0x04, sideData, pos, protection.getFileSize());
		protection.notify(fileData);

		side.push_back(fileHeader);
		side.push_back(fileData);
	}
	} catch(...) { pos =lastPos; }

	// Process post-file data
	if (protection.epiloguePresent()) {
		FDSBlockWithSum epilogueData(protection.epilogueType(), sideData, pos, protection.epilogueSize());
		protection.notify(epilogueData);
		side.push_back(epilogueData);
	}
	
	data.push_back(side);
}

std::vector<uint8_t> FDSFile_Venus::outputSide(int side) const {
	std::vector<uint8_t> result(3);
	for (auto& block: data.at(side)) {
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());		
		result.push_back(block.sum &0xFF);
		result.push_back(block.sum >>8);
	}
	result[0] =((result.size() -3) >> 0) &0xFF;
	result[1] =((result.size() -3) >> 8) &0xFF;
	result[2] =((result.size() -3) >>16) &0xFF;
	return result;
}