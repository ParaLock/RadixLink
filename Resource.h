#pragma once

#include "JobInfo.h"
#include "Buffer.h"

struct Resource {
	
	std::string   		destManager;
	
	char               	target[50];

	unsigned int  		jobID;
	int           		type;
	bool  				isOutgoing;

	JobInfo 	info;
	std::string codeFn;
	std::string dataFn;
	Buffer      buff;
	Buffer      result;

	Resource() {
		
		isOutgoing = true;
	}
	
	void init(const Resource& other) {
		info = other.info;
		jobID = other.jobID;
		type = other.type;
		
		buff = other.buff;
		codeFn = other.codeFn;
		response = other.response;

		codeFn = other.codeFn;

		destManager = other.destManager;

		for(int i = 0; i < 20; i++) {
			target[i] = other.target[i];
		}
	}

	Resource(const Resource& other) {
	
		init(other);
	}
	
	Resource(const Resource&& other) {
	
		init(other);
	}
};

