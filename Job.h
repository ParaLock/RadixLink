#pragma once

#include "windows.h"

#include "Resource.h"
#include "JobInfo.h"
#include "Encoder.h"

struct Job {
	
	typedef void (*FUNC)(char *, size_t, Buffer& out);
	
	std::vector<int> _preReqs;
	std::map<int, Resource> m_resources;
	
	std::vector<Resource> m_results;

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
		
		
	}

	void addPreReq(int type) {

		std::cout << "Job: add prereq " << type << std::endl;

		_preReqs.push_back(type);
	}
	
	//Returns true if job setup is finalized
	bool addResource(Resource& resource) {
		
		std::cout << "Job: Adding Resource!" << std::endl;

		if(resource.type == RESOURCE_TYPE_RESULT) {
			
			resource.destManager = "net_manager";
			m_results.push_back(resource);

		} else {

			m_resources.insert({resource.type, resource});
		
		}

		removePreReq(resource.type);

		return true;
	}
	
	void execute() {
	
		if(_preReqs.size() == 0) {

			Resource& codeRes = m_resources.at(RESOURCE_TYPE_CODE);
			Resource& dataRes = m_resources.at(RESOURCE_TYPE_DATA);
			Resource& infoRes = m_resources.at(RESOURCE_TYPE_JOB);

			std::cout << "Job: filename: " << codeRes.codeFn << std::endl;
			std::cout << "Job: job name: ";

			for(int i = 0; i < 20; i++) {
				std::cout << infoRes.info.jobName[i];
			}

			std::cout << std::endl;

			HMODULE dllHandle = LoadLibraryA(codeRes.codeFn.c_str());

			FUNC func;

			func = (FUNC) GetProcAddress(dllHandle, std::string(infoRes.info.jobName).c_str());
			
			std::cout << "****************************RUNNING JOB!!!!**********************" << std::endl;
 
			func(dataRes.buff.getBase(), dataRes.buff.getSize(), m_results[0].result);
			
			std::cout << "*****************************************************************" << std::endl;

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
	
	std::vector<Resource>& getResults() {
		
		return m_results;	
	}
};


