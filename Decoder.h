#pragma once

#include "Encoder.h"
#include "Buffer.h"
#include "Resource.h"

#include <vector>
#include <fstream>
#include <functional>
#include <map>

struct Decoder {

	std::map<int, std::function<void(EncoderHeader*, char*, Resource&)>> m_decoders;

	void registerHandler(int type, std::function<void(EncoderHeader*, char*, Resource&)> handler) {
		m_decoders.insert({type, handler});
	}

	bool run(Buffer& buff, std::vector<Resource>& resources) {
		
		if(buff.getSize() == 0) return true;

		size_t size = buff.getSize(); 
		
		EncoderHeader* header = (EncoderHeader*)buff.getBase();

		while(size) {
			
			Resource jobRes;
			
			jobRes.type  = header->type;
			jobRes.jobID = header->jobID;

			char* payload = (char*)header + sizeof(EncoderHeader);
			
			std::cout << "Decoder: Decoded Resource Type: " << jobRes.type << std::endl;

			m_decoders.at(jobRes.type)(header, payload, jobRes);
							
			resources.push_back(std::move(jobRes));

			size  -= sizeof(EncoderHeader) + header->payloadSize;
			header = (EncoderHeader*)(payload + header->payloadSize);
			
		}
		
		return true;
	}
	

};