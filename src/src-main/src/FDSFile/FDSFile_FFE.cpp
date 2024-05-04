#include "FDSFile_FFE.hpp"
#include "FDSFile_FFE_dump.hpp"
#include "to_string.hpp"

FDSBlockFFE::FDSBlockFFE(const uint8_t wantedType, const std::vector<uint8_t>& input, uint32_t& pos, size_t size):
	FDSBlock() {
	CRC16 crc16;
	crc16.add(0x80);
	crc16.add(wantedType);
	type =wantedType;
	sum =wantedType;
	
	while (size--) {
		crc16.add(input.at(pos));
		sum +=input.at(pos);
		data.push_back(input.at(pos));
		pos++;
	}
	crc16.add(0x00);
	crc16.add(0x00);
	crc =crc16.get();

	uint8_t foundCRC =input.at(pos++);
	if (foundCRC !=(crc &0xFF)) EI.DbgOut(L"Wrong CRC (%04X instead of %04X) at position $%X\n", foundCRC, crc >>8, pos);
	crc =crc &0xFF00 | foundCRC;
}


bool FDSFile_FFE::detect(const std::vector<uint8_t>& diskData) {
	return diskData.size() ==0x20208 && diskData[0] ==0xC0 && diskData[1] ==0x00;
}

std::wstring FDSFile_FFE::getNameForSide (const std::wstring& base, const int side) const {
	std::wstring result =base;
	if (data.size() >2) {
		result +='-';
		result +=('0' +side);
	}
	result +=L".ffe";
	return result;
}

FDSFile_FFE::FDSFile_FFE (const std::vector<uint8_t>& diskData, FDSProtection& protection) {
	catalogue =diskData[0x1E208 +0x1A01] *100 +diskData[0x1E208 +0x1A02] *10 +diskData[0x1E208 +0x1A03];
	unsigned int numberOfSides =diskData[0x1E208 +0x1A00] +1;
	
	for (unsigned int i =0; i <numberOfSides; i++) {
		std::vector<FDSBlock> side;
		std::vector<uint8_t> sideData(diskData.begin() +i*65536 +0x0208, diskData.begin() +i*65536 +0x0208 +65536);
		uint32_t pos =0;

		unsigned int filesLeft =sideData[pos++];
		
		FDSBlockFFE diskHeader(0x01, sideData, pos, 55);
		protection.notify(diskHeader);
		side.push_back(diskHeader);

		FDSBlockFFE fileCount(0x02, sideData, pos, 1);
		protection.notify(fileCount);
		side.push_back(fileCount);

		while(filesLeft--) {
			FDSBlockFFE fileHeader(0x03, sideData, pos, 15);
			protection.notify(fileHeader);

			FDSBlockFFE fileData(0x04, sideData, pos, protection.getFileSize());
			protection.notify(fileData);

			side.push_back(fileHeader);
			side.push_back(fileData);
		}
		// No hidden files here, since the data contained the actual number of files.

		// Process post-file data
		if (protection.epiloguePresent()) {
			FDSBlockFFE epilogueData(protection.epilogueType(), sideData, pos, protection.epilogueSize());
			protection.notify(epilogueData);
			side.push_back(epilogueData);
		}
		
		data.push_back(side);
	}
}

std::vector<uint8_t> FDSFile_FFE::outputSide(int side) const {
	std::vector<uint8_t> result(1, 0); // Dummy byte for actual file count

	if ((unsigned) side <data.size()) { // To get to an even side count, outputSide maybe called to output one more side than the image actually has
		unsigned int numberOfFiles =0;
		for (auto& block: data.at(side)) {
			result.insert(result.end(), block.data.begin(), block.data.end());
			result.push_back(block.crc &0xFF);
			if (block.type ==0x03) numberOfFiles++;
		}
		result[0] =numberOfFiles;
	}

	unsigned int maxSideLength =side &1? 0xE000: 0x10000; // The odd-numbered side must have the 8 KiB footer code
	if (result.size() >maxSideLength) throw std::out_of_range("Side longer than " +std::to_string(maxSideLength) +" bytes: cannot output to FFE format");

	while (result.size() <maxSideLength) result.push_back(0xFF);
	if (side &1) for (auto& c: ffeFooter) result.push_back(c);
	return result;
}

std::vector<std::vector<uint8_t> > FDSFile_FFE::outputSides() const {
	std::vector<std::vector<uint8_t> > result;

	std::vector<uint8_t> file; // two sides per file
	unsigned int sides =data.size() +(data.size() &1); // side count must be even; outputSide() will fill additional side with FF
	for (unsigned int side =0; side <sides; side++) {
		if (~side &1) {
			file.clear();
			for (auto& c: ffeHeader) file.push_back(c);
		}
		std::vector<uint8_t> sideData =outputSide(side);
		file.insert(file.end(), sideData.begin(), sideData.end());
		if (side &1) {
			file[0x1E208 +0x1A00] =side ==data.size()? 0: 1; // denote if the last file should display A or A+B
			file[0x1E208 +0x1A01] =(catalogue /100) %10;
			file[0x1E208 +0x1A02] =(catalogue / 10) %10;
			file[0x1E208 +0x1A03] = catalogue       %10;
			result.push_back(file);
		}
	}
	return result;
}
