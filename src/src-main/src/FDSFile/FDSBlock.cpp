#include "FDSFile.hpp"

// ---- CRC16
CRC16::CRC16(): crc(0) {
}

uint16_t CRC16::get() const {
	return crc &0xFFFF;
}

void CRC16::add (uint8_t data) {
	crc |=data <<16;
	for (int i =0; i <8; i++) {
		if (crc &1) crc ^=0x10810;
		crc >>=1;
	}
}

FDSBlock::FDSBlock():
	type(0) {
}

FDSBlock::FDSBlock(const uint8_t wantedType, const std::vector<uint8_t>& input, uint32_t& pos, size_t size):
	FDSBlock() {
	type =input.at(pos++);
	if (type !=wantedType) throw std::runtime_error("Wrong block type " +std::to_string((int) type) +" instead of " +std::to_string((int) wantedType) +" at side offset " +std::to_string(pos -1));
	CRC16 crc16;
	crc16.add(0x80);
	crc16.add(type);
	sum =type;
	
	while (size--) {
		crc16.add(input.at(pos));
		sum +=input.at(pos);
		data.push_back(input.at(pos));
		pos++;
	}
	crc16.add(0x00);
	crc16.add(0x00);
	crc =crc16.get();
}

FDSBlockWithCRC::FDSBlockWithCRC(const uint8_t wantedType, const std::vector<uint8_t>& input, uint32_t& pos, size_t size):
	FDSBlock(wantedType, input, pos, size) {
	uint16_t foundCRC =input.at(pos +0) | input.at(pos +1) <<8;
	pos +=2;
	if (foundCRC !=crc) EI.DbgOut(L"Wrong CRC (%04X instead of %04X) at position $%X\n", foundCRC, crc, pos);
	crc =foundCRC;
}

uint8_t FDSBlockMFM::getByte (const std::vector<bool>& mfm, uint32_t pos) {
	return getNextByte(mfm, pos); // Caller's "pos" is unchanged because it is passed by value
}

uint8_t FDSBlockMFM::getNextByte (const std::vector<bool>& mfm, uint32_t& pos) {
	uint8_t result =0;
	for (int i =0; i <8; i++) {
		result >>=1;
		result |=mfm.at(pos)? 0x80: 00;
		pos +=2;
	}
	return result;
}

FDSBlockMFM::FDSBlockMFM(const uint8_t wantedType, const std::vector<bool>& input, uint32_t& pos, size_t size):
	FDSBlock() {
	pos +=18*16; // Minimum pre-gap size

	if (wantedType ==1)
		while (getByte(input, pos +16*0) !=0x80 ||
	               getByte(input, pos +16*1) !=0x01 ||
		       getByte(input, pos +16*2) !='*'    // Don't search for actual NINTENDO-HVC, because FFE RTS disks only have *
		       ) pos++; // Search for disk header
	else
		while (getByte(input, pos) !=0x80) pos++; // Search for gap end mark
	CRC16 crc16;
	crc16.add(getNextByte(input, pos));
	
	type =getNextByte(input, pos);
	if (type !=wantedType) throw std::runtime_error("Wrong block type " +std::to_string((int) type) +" instead of " +std::to_string((int) wantedType) +" at MFM position " +std::to_string(pos -1));
	crc16.add(type);
	sum =type;
	
	while (size--) {
		uint8_t byte =getNextByte(input, pos);
		crc16.add(byte);
		sum +=byte;
		data.push_back(byte);
	}
	crc16.add(0x00);
	crc16.add(0x00);
	crc  =getNextByte(input, pos);
	crc |=getNextByte(input, pos) <<8;;
	if (crc !=crc16.get()) EI.DbgOut(L"Wrong CRC (%04X instead of %04X) at MFM position %d\n", crc, crc16.get(), pos);
}

