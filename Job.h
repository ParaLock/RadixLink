#pragma once

#include "windows.h"

#include "Resource.h"
#include "JobInfo.h"
#include "Encoder.h"

struct Job {
	
	typedef void (*FUNC)(char *, size_t, Buffer& out);

	std::map<int, Resource>    m_resources;
	
	std::vector<Resource>      m_results;

	bool        _isComplete;

	bool        _hasCode;
	bool        _hasData;
	bool        _hasJob;

	Job() {
		
		_hasCode = false;
		_hasData = false;
		_hasJob  = false;

		_isComplete = false;
	}
	
	//Returns true if job setup is finalized
	bool addResource(Resource& resource) {
		
		std::cout << "Job: Adding Resource!" << std::endl;

		//yup... this is wicked gross.. TODO: Implement working generic prereq system.
		if(resource.type == RESOURCE_TYPE_RESULT) {
			
			resource.destManager = "net_manager";
			m_results.push_back(resource);

		} else if(resource.type == RESOURCE_TYPE_CODE) {

			_hasCode = true;

			m_resources.insert({resource.type, resource});
		
		} else if(resource.type == RESOURCE_TYPE_DATA) {

			m_resources.insert({resource.type, resource});

			_hasData = true;

		} else if(resource.type == RESOURCE_TYPE_JOB) {

			m_resources.insert({resource.type, resource});

			_hasJob = true;
		}

		return true;
	}
	
	void execute() {
	
		if(isRunnable()) {

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

		return _hasCode && _hasData && _hasJob;
	}
	
	std::vector<Resource>& getResults() {
		
		return m_results;	
	}
};


