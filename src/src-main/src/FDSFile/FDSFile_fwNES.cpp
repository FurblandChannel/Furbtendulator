#include "FDSFile_fwNES.hpp"
#define ALLOW_OVERSIZED_INPUT 1

bool FDSFile_fwNES::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=16 &&
	       diskData.at(0) =='F' &&
	       diskData.at(1) =='D' &&
	       diskData.at(2) =='S' &&
	       diskData.at(3) ==0x1A &&
	#if ALLOW_OVERSIZED_INPUT
	       diskData.size() >=(unsigned) diskData.at(4) *65500 +16;
	#else
	       diskData.size() ==(unsigned) diskData.at(4) *65500 +16;
	#endif
}

bool FDSFile_fwNES_Headerless::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() >=65500 && (diskData.size() %65500) ==0;
}

std::wstring FDSFile_fwNES::getNameForSide (const std::wstring& base, const int) const {
	return base +L".fds";
}

FDSFile_fwNES::FDSFile_fwNES (const std::vector<uint8_t>& diskData, FDSProtection& protection) {
	unsigned int headerSize =diskData.at(0) =='F' && diskData.at(1) =='D' && diskData.at(2) =='S' && diskData.at(3) ==0x1A? 16: 0;
	unsigned int numberOfSides =headerSize ==16? diskData.at(4): diskData.size() /65500;
	
	for (unsigned int i =0; i <numberOfSides; i++) {
		std::vector<FDSBlock> side;

		#if ALLOW_OVERSIZED_INPUT
		std::vector<uint8_t> sideData(diskData.begin() +i*65500 +headerSize, diskData.end());
		while(!(sideData.at(0) ==0x01 && sideData.at(1) =='*' && sideData.at(2) =='N' && sideData.at(3) =='I' && sideData.at(4) =='N')) sideData.erase(sideData.begin());
		#else
		std::vector<uint8_t> sideData(diskData.begin() +i*65500 +headerSize, diskData.begin() +i*65500 +headerSize +65500);
		#endif
		uint32_t pos =0;

		FDSBlock diskHeader(0x01, sideData, pos, 55);
		protection.notify(diskHeader);
		side.push_back(diskHeader);

		FDSBlock fileCount(0x02, sideData, pos, 1);
		protection.notify(fileCount);
		side.push_back(fileCount);

		unsigned int filesLeft =fileCount.data.at(0);
		while(filesLeft--) {
			FDSBlock fileHeader(0x03, sideData, pos, 15);
			protection.notify(fileHeader);

			FDSBlock fileData(0x04, sideData, pos, protection.getFileSize());
			protection.notify(fileData);

			side.push_back(fileHeader);
			side.push_back(fileData);
		}

		uint32_t lastPos;
		try {	// Process any hidden files
		while(1) {
			lastPos =pos;
			FDSBlock fileHeader(0x03, sideData, pos, 15);
			protection.notify(fileHeader);

			lastPos =pos;
			FDSBlock fileData(0x04, sideData, pos, protection.getFileSize());
			protection.notify(fileData);

			side.push_back(fileHeader);
			side.push_back(fileData);
		}
		} catch(...) { pos =lastPos; }
		
		// Process post-file data
		if (protection.epiloguePresent()) {
			// Workaround for buggy No-Intro "Quick Hunter" image, lacking the $00 block type byte
			if (protection.epilogueType() ==0x00 && sideData[pos] !=0x00) sideData[--pos] =0x00;
			
			FDSBlock epilogueData(protection.epilogueType(), sideData, pos, protection.epilogueSize());
			protection.notify(epilogueData);
			side.push_back(epilogueData);
		}
		
		data.push_back(side);
	}
}

std::vector<uint8_t> FDSFile_fwNES::outputSide(int side) const {
	std::vector<uint8_t> result;
	for (auto& block: data.at(side)) {
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());
	}
	while (result.size() <65500) result.push_back(0);
	if (result.size() >65500) throw std::out_of_range("Side longer than 65500 bytes: cannot output to fwNES format");
	return result;
}

std::vector<std::vector<uint8_t> > FDSFile_fwNES::outputSides() const {
	std::vector<std::vector<uint8_t> > result;

	std::vector<uint8_t> file;
	file.push_back('F');
	file.push_back('D');
	file.push_back('S');
	file.push_back(0x1A);
	file.push_back(data.size());
	file.resize(16);

	for (unsigned int side =0; side <data.size(); side++) {
		std::vector<uint8_t> sideData =outputSide(side);
		file.insert(file.end(), sideData.begin(), sideData.end());
	}

	result.push_back(file);
	return result;
}

std::vector<std::vector<uint8_t> > FDSFile_fwNES_Headerless::outputSides() const {
	std::vector<std::vector<uint8_t> > result;

	std::vector<uint8_t> file;
	for (unsigned int side =0; side <data.size(); side++) {
		std::vector<uint8_t> sideData =outputSide(side);
		file.insert(file.end(), sideData.begin(), sideData.end());
	}

	result.push_back(file);
	return result;
}