#pragma once

#include "windows.h"

#include "Resource.h"
#include "JobInfo.h"
#include "Encoder.h"

struct Job {
	
	typedef void (*FUNC)(char *, size_t, Buffer& out);
	
	std::vector<int> _preReqs;
	
	JobInfo 	_info;
	std::string _codeFn;
	Buffer      _genericData;
	
	Buffer      _result;
	
	bool        _isComplete;
	
	Job() {
		
		_isComplete = false;
	}
	
	void removePreReq(int type) {
		
		std::cout << "Job: remove prereq called with " <<  type << " current num prereqs " << _preReqs.size() << std::endl;

		for(int i = 0; i < _preReqs.size(); i++) {
			
			std::cout << "Job: checking prereq: " <<  type << std::endl;


			if(_preReqs[i] == type) {
								
				std::cout << "Job: removed prereq: " <<  _preReqs[i] << std::endl;

				_preReqs.erase(_preReqs.begin() + i);

				break;
			}
		}
		
		if(_preReqs.size() == 0) {
		
			_isComplete = true;
		}
	}

	void addPreReq(int type) {

		std::cout << "Job: add prereq " << type << std::endl;

		_preReqs.push_back(type);
	}
	
	//Returns true if job setup is finalized
	bool addResource(Resource& resource) {
		
		std::cout << "Job: Adding Resource!" << std::endl;

		if(resource.type == RESOURCE_TYPE_JOB) {

			_info = resource.info;
		}

		if(resource.type == RESOURCE_TYPE_CODE) {

			_codeFn = resource.codeFn;

			std::cout << "Job: filename: " << _codeFn << std::endl;
		}

		if(resource.type == RESOURCE_TYPE_DATA) {

			_genericData = resource.buff;
		}
		
		removePreReq(resource.type);

		return true;
	}
	
	void execute() {
	
		if(_preReqs.size() == 0) {

			std::cout << "Job filename: " << _codeFn << std::endl;

			HMODULE dllHandle = LoadLibraryA(_codeFn.c_str());

			FUNC func;

			func = (FUNC) GetProcAddress(dllHandle, std::string(_info.jobName).c_str());

			func(_genericData.getBase(), _genericData.getSize(), _result);
			
			_isComplete = true;
			
			FreeLibrary(dllHandle);
		}
	}
	
	bool isComplete() {
		
		return _isComplete;
	}

	bool isRunnable() {

		if(_preReqs.size() == 0) {

			return true;	
		} 

		return false;
	}
	
	Buffer& getResult() {
		
		return _result;	
	}
};


