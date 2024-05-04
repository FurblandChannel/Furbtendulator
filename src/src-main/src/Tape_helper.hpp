const char* empty(const char *s) {
	if (s ==NULL)
		return "";
	else
		return s;
}

uint32_t lsb32(std::vector<uint8_t>::const_iterator data) {
	return data[0] | (data[1] <<8) | (data[2] <<16) | (data[3] <<24);
}

uint32_t msb32(std::vector<uint8_t>::const_iterator data) {
	return data[3] | (data[2] <<8) | (data[1] <<16) | (data[0] <<24);
}

void putlsb16(uint8_t* data, const uint16_t value) {
	data[0] =(value >> 0) &0xFF;
	data[1] =(value >> 8) &0xFF;
}

void putlsb32(uint8_t* data, const uint32_t value) {
	data[0] =(value >> 0) &0xFF;
	data[1] =(value >> 8) &0xFF;
	data[2] =(value >>16) &0xFF;
	data[3] =(value >>24) &0xFF;
}

std::string getid(std::vector<uint8_t>::const_iterator data) {
	std::string result;
	result.push_back(data[0]);
	result.push_back(data[1]);
	result.push_back(data[2]);
	result.push_back(data[3]);
	return result;
}

void addRIFFHeader(std::vector<uint8_t>& data, const std::string& id, const std::string& subid) {
	// To minimize the amount of insert operations, create the header first in a separate vector
	std::vector<uint8_t> header;
	header.insert(header.end(), id.begin(), id.end());
	size_t chunkSize =data.size() +subid.size();
	header.push_back(chunkSize >> 0 &0xFF);
	header.push_back(chunkSize >> 8 &0xFF);
	header.push_back(chunkSize >>16 &0xFF);
	header.push_back(chunkSize >>24 &0xFF);
	header.insert(header.end(), subid.begin(), subid.end());
	
	// Then insert the entire header in front of the data
	data.insert(data.begin(), header.begin(), header.end());
	
	// Align to WORD boundary, if necessary, without increasing the size field
	if (header.size() &1) header.push_back(0x00);
}

void addRIFFHeader(std::vector<uint8_t>& data, const std::string& id) {
	addRIFFHeader(data, id, "");
}

std::vector<uint8_t> makeWAVEFile8Bit (const std::vector<uint8_t>& data, uint32_t rate, uint8_t channels) {
	// Create "fmt " chunk
	std::vector<uint8_t> fmtChunk(16);
	putlsb16(&fmtChunk[ 0], 1);                  // Format type: PCM
	putlsb16(&fmtChunk[ 2], channels);           // # of channels
	putlsb32(&fmtChunk[ 4], rate);               // # of samples per second
	putlsb32(&fmtChunk[ 8], rate *channels *1);  // # of average bytes per second
	putlsb16(&fmtChunk[12], 1);                  // Block alignment
	putlsb16(&fmtChunk[14], 8);                 // Bits per sample
	addRIFFHeader(fmtChunk, "fmt ");

	// Create "data" chunk, ensuring that it is converted to little-endian values
	std::vector<uint8_t> result =data;
	addRIFFHeader(result, "data");
	
	// Insert the "fmt " chunk at the beginning of the result
	result.insert(result.begin(), fmtChunk.begin(), fmtChunk.end());

	// Add the RIFF header
	addRIFFHeader(result, "RIFF", "WAVE");
	
	return result;
}
