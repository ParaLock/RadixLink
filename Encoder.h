#pragma once

#include <string>
#include "Buffer.h"
#include "Resource.h"

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <map>
#include <functional>

const int RESOURCE_TYPE_CODE = 0;
const int RESOURCE_TYPE_DATA = 1;
const int RESOURCE_TYPE_JOB  = 2;
const int RESOURCE_TYPE_RESULT = 3;
const int RESOURCE_TYPE_STATUS = 4;

struct EncoderHeader {
	
	unsigned int type;
	unsigned int jobID;
	unsigned int payloadSize;
	unsigned int jobType;
	
}__attribute__((packed));

struct Encoder {

	std::map<int, std::function<void(Buffer&, Resource&)>> m_encoders;

	void registerHandler(unsigned int type, std::function<void(Buffer&, Resource&)> handler) {

		m_encoders.insert({type, handler});
	}

	bool run(Buffer& buff, std::vector<Resource>& resources) {

		for(int i = 0; i < resources.size(); i++) {

			run(buff, resources[i]);	
		}

		return true;
	}

	bool run(Buffer& buff, Resource& resource) {
		
		m_encoders.at(resource.type)(buff, resource);
		
		return true;
	}

	static bool run(char* src, unsigned int size, unsigned int jobID, unsigned int jobType, unsigned int type, Buffer& buff) {
	
		EncoderHeader header;
		header.type = type;
		header.jobID = jobID;
		header.jobType = jobType;
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
			ifs.read(buff.getCurrentOffsetPtr() - pos, pos);

		} else {
			
			std::cout << "Encoder: File open failed!" << std::endl;
			
			return false;
		}

		
		return true;
	}
};