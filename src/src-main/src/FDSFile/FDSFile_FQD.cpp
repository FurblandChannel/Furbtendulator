#include "FDSFile_FQD.hpp"

bool FDSFile_FQD::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=16 &&
	       diskData.at(0) =='F' &&
	       diskData.at(1) =='Q' &&
	       diskData.at(2) =='D' &&
	       diskData.at(3) ==0x1A;
}

std::wstring FDSFile_FQD::getNameForSide (const std::wstring& base, const int) const {
	return base +L".fqd";
}

FDSFile_FQD::FDSFile_FQD (const std::vector<uint8_t>& diskData) {
	targetDevice =diskData.at(5);
	targetConfig =diskData.at(6);
	expDevice    =diskData.at(15);
	uint32_t sideStart =16;
	for (int i =0; i <diskData.at(4); i++) {
		uint32_t sideSize =diskData.at(sideStart +0) | 
		                   diskData.at(sideStart +1) <<8 |
		                   diskData.at(sideStart +2) <<16 |
		                   diskData.at(sideStart +3) <<24
		;
		sideStart +=4;
		std::vector<uint8_t> sideData(diskData.begin() +sideStart, diskData.begin() +sideStart +sideSize);
		sideStart +=sideSize;

		uint32_t pos =0;
		std::vector<FDSBlock> side;
		
		while (pos <sideSize) {
			uint32_t blockSize =sideData.at(pos +0) | 
			                    sideData.at(pos +1) <<8 |
			                    sideData.at(pos +2) <<16 |
			                    sideData.at(pos +3) <<24
			;
			pos +=4;
			FDSBlock block(sideData.at(pos +0), sideData, pos, blockSize -1);
			side.push_back(block);
		}
		
		data.push_back(side);
	}
}

std::vector<uint8_t> FDSFile_FQD::outputSide(int side) const {
	std::vector<uint8_t> result;
	for (auto& block: data.at(side)) {
		uint32_t blockSize =block.data.size() +1; // Include block type byte
		result.push_back(blockSize >>0  &0xFF);
		result.push_back(blockSize >>8  &0xFF);
		result.push_back(blockSize >>16 &0xFF);
		result.push_back(blockSize >>24 &0xFF);
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());
	}
	return result;
}

std::vector<std::vector<uint8_t> > FDSFile_FQD::outputSides() const {
	std::vector<std::vector<uint8_t> > result;

	std::vector<uint8_t> file;
	file.push_back('F');
	file.push_back('Q');
	file.push_back('D');
	file.push_back(0x1A);
	file.push_back(data.size());
	file.push_back(targetDevice);
	file.push_back(targetConfig);
	file.resize(15);
	file.push_back(expDevice);

	for (unsigned int side =0; side <data.size(); side++) {
		std::vector<uint8_t> sideData =outputSide(side);
		file.push_back(sideData.size() >>0  &0xFF);
		file.push_back(sideData.size() >>8  &0xFF);
		file.push_back(sideData.size() >>16 &0xFF);
		file.push_back(sideData.size() >>24 &0xFF);
		file.insert(file.end(), sideData.begin(), sideData.end());
	}

	result.push_back(file);
	return result;
}