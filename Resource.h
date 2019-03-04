#pragma once

#include "JobInfo.h"
#include "Buffer.h"

struct Resource {
	
	std::string   		destManager;
	
	char               	senderName[50];

	unsigned int  		jobID;
	int           		type;

	JobInfo 	info;
	std::string codeFn;
	Buffer      buff;
	
	struct Response {
	
		std::string sender;
		size_t      reponseSize;
		
	} response;
	
	Resource() {
		
	}
	
	Resource(const Resource& other) {
	
		info = other.info;
		jobID = other.jobID;
		type = other.type;
		
		buff = other.buff;
		
		response = other.response;

		destManager = other.destManager;
	}
	
	Resource(const Resource&& other) {
	
		info = other.info;
		jobID = other.jobID;
		type = other.type;
		
		buff = other.buff;
		
		response = other.response;

		destManager = other.destManager;
	}
};

