#pragma once

#include <string>
#include "Buffer.h"
#include "Resource.h"

#include <fstream>
#include <iostream>
#include <map>
#include <functional>

const int RESOURCE_TYPE_CODE = 0;
const int RESOURCE_TYPE_DATA = 1;
const int RESOURCE_TYPE_JOB  = 2;
const int RESOURCE_TYPE_RESULT = 3;

struct EncoderHeader {
	

	int          type;
	unsigned int jobID;
	size_t       payloadSize;
	
};

struct Encoder {

	std::map<int, std::function<void(Buffer&, Resource&)>> m_encoders;

	void registerHandler(int type, std::function<void(Buffer&, Resource&)> handler) {

		m_encoders.insert({type, handler});
	}

	bool run(Buffer& buff, std::vector<Resource>& resources) {


		for(int i = 0; i < resources.size(); i++) {

			m_encoders.at(resources[i].type)(buff, resources[i]);
		}

		return true;
	}

	static bool run(char* src, size_t size, unsigned int jobID, int type, Buffer& buff) {
	
		EncoderHeader header;
		header.type = type;
		header.jobID = jobID;
		header.payloadSize = size;
		
		buff.write((char*)&header, sizeof(EncoderHeader));
		buff.write(src, size);
		
		return true;
	}
	
	static bool run(std::string fn, Buffer& buff) {
	
		std::ifstream ifs(fn, std::ios::binary|std::ios::ate);
		
		if(ifs) {
			

			std::ifstream::pos_type pos = ifs.tellg();

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