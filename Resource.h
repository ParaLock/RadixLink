#pragma once

#include "JobInfo.h"
#include "Buffer.h"

struct Resource {
	
	std::string   		destManager;
	
	char               	target[50];
	unsigned int        jobType;	

	unsigned int  		jobID;
	unsigned int        type;

	unsigned int        order;

	JobInfo 	info;
	std::string codeFn;
	std::string dataFn;

	Buffer      buff;

	Resource() {
		
		jobID = 0;
		type  = -1;
		order = 0;
		jobType = -1;
		destManager = "";
		buff.clear();

		target[0] = 'a';
		target[1] = 'l';
		target[2] = 'l';
		target[3] = '\0';
	}
	
	void init(const Resource& other) {
		info = other.info;
		jobID = other.jobID;
		type = other.type;
		jobType = other.jobType;
		
		buff = other.buff;
		codeFn = other.codeFn;
		dataFn = other.dataFn;

		//result = other.result;

		order = other.order;

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

