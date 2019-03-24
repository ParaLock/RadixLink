#pragma once

#include "JobInfo.h"
#include "Buffer.h"

struct Resource {
	
	std::string   		destManager;
	
	char               	target[50];

	unsigned int  		jobID;
	unsigned int        type;

	JobInfo 	info;
	std::string codeFn;
	std::string dataFn;
	Buffer      buff;
	Buffer      result;

	Resource() {
		
		target[0] = 'a';
		target[1] = 'l';
		target[2] = 'l';
		target[3] = '\0';
	}
	
	void init(const Resource& other) {
		info = other.info;
		jobID = other.jobID;
		type = other.type;
		
		buff = other.buff;
		codeFn = other.codeFn;
		dataFn = other.dataFn;

		result = other.result;

		destManager = other.destManager;

		for(int i = 0; i < 50; i++) {
			target[i] = other.target[i];
		}
	}

	Resource(const Resource& other) {
	
		init(other);
	}
	
	Resource(const Resource&& other) {
	
		init(other);
	}

	void operator=(const Resource& other) {

		init(other);
	}
};

