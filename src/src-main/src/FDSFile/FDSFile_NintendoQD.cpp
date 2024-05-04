#include "FDSFile_NintendoQD.hpp"

bool FDSFile_NintendoQD::detect(const std::vector<uint8_t>& diskData) {
	std::string compare("\x01*NINTENDO-HVC*");
	return diskData.size() >=65536 &&
	       (diskData.size() %65536) ==0 &&
	       std::equal(diskData.begin(), diskData.begin() +15, compare.begin(), compare.end());
}

std::wstring FDSFile_NintendoQD::getNameForSide (const std::wstring& base, const int) const {
	return base +L".qd";
}

FDSFile_NintendoQD::FDSFile_NintendoQD (const std::vector<uint8_t>& diskData, FDSProtection& protection) {
	for (unsigned int i =0; i <diskData.size() /65536; i++) {
		std::vector<FDSBlock> side;
		std::vector<uint8_t> sideData(diskData.begin() +i*65536, diskData.begin() +i*65536 +65536);
		uint32_t pos =0;

		FDSBlockWithCRC diskHeader(0x01, sideData, pos, 55);
		protection.notify(diskHeader);
		side.push_back(diskHeader);

		FDSBlockWithCRC fileCount(0x02, sideData, pos, 1);
		protection.notify(fileCount);
		side.push_back(fileCount);

		unsigned int filesLeft =fileCount.data.at(0);
		while(filesLeft--) {
			FDSBlockWithCRC fileHeader(0x03, sideData, pos, 15);
			protection.notify(fileHeader);

			FDSBlockWithCRC fileData(0x04, sideData, pos, protection.getFileSize());
			protection.notify(fileData);

			side.push_back(fileHeader);
			side.push_back(fileData);
		}

		uint32_t lastPos;
		try {	// Process any hidden files
		while(1) {
			lastPos =pos;
			FDSBlockWithCRC fileHeader(0x03, sideData, pos, 15);
			protection.notify(fileHeader);

			lastPos =pos;
			FDSBlockWithCRC fileData(0x04, sideData, pos, protection.getFileSize());
			protection.notify(fileData);

			side.push_back(fileHeader);
			side.push_back(fileData);
		}
		} catch(...) { pos =lastPos; }

		// Process post-file data
		if (protection.epiloguePresent()) {
			FDSBlockWithCRC epilogueData(protection.epilogueType(), sideData, pos, protection.epilogueSize());
			protection.notify(epilogueData);
			side.push_back(epilogueData);
		}
		
		data.push_back(side);
	}
}

std::vector<uint8_t> FDSFile_NintendoQD::outputSide(int side) const {
	std::vector<uint8_t> result;
	for (auto& block: data.at(side)) {
		result.push_back(block.type);
		result.insert(result.end(), block.data.begin(), block.data.end());
		result.push_back(block.crc &0xFF);
		result.push_back(block.crc >>8);
	}
	while (result.size() <65536) result.push_back(0);
	if (result.size() >65536) throw std::out_of_range("Side longer than 65536 bytes: cannot output to Nintendo QD format");
	return result;
}

std::vector<std::vector<uint8_t> > FDSFile_NintendoQD::outputSides() const {
	std::vector<std::vector<uint8_t> > result;

	std::vector<uint8_t> file;
	for (unsigned int side =0; side <data.size(); side++) {
		std::vector<uint8_t> sideData =outputSide(side);
		file.insert(file.end(), sideData.begin(), sideData.end());
	}

	result.push_back(file);
	return result;
}