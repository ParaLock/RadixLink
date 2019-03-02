#pragma once

#include <string>
#include "Buffer.h"

#include <fstream>


const int RESOURCE_TYPE_CODE = 0;
const int RESOURCE_TYPE_DATA = 1;
const int RESOURCE_TYPE_JOB  = 2;


struct EncoderHeader {
	

	int          type;
	unsigned int jobID;
	size_t       payloadSize;
	
};

struct Encoder {

	bool run(char* src, size_t size, unsigned int jobID, int type, Buffer& buff) {
	
		EncoderHeader header;
		header.type = type;
		header.jobID = jobID;
		header.payloadSize = size;
		
		buff.write((char*)&header, sizeof(EncoderHeader));
		buff.write(src, size);
		
		return true;
	}
	
	bool run(std::string fn, unsigned int jobID, int type, Buffer& buff) {
	
		EncoderHeader header;
		header.type = type;
		header.jobID = jobID;
	
		std::ifstream ifs(fn, std::ios::binary|std::ios::ate);
		
		if(ifs) {
			

			std::ifstream::pos_type pos = ifs.tellg();

			header.payloadSize = pos;

			buff.write((char*)&header, sizeof(EncoderHeader));

			for(int i = 0; i < pos; i++) {

				char junk = 'f';
				buff.write(&junk, sizeof(char));
			}


			ifs.seekg(0, std::ios::beg);
			ifs.read(buff.getCurrentOffset() - pos, pos);

		} else {
			
			std::cout << "Encoder: File open failed!" << std::endl;
					
		}

		
		return true;
	}
};