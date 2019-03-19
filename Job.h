#pragma once

#include "windows.h"

#include "Resource.h"
#include "JobInfo.h"
#include "Encoder.h"

struct Job {
	
	typedef void (*FUNC)(char *, size_t, Buffer& out);

	std::map<int, Resource>    m_resources;
	std::vector<Resource>      m_results;

	Resource 				   m_result;

	int id;

	bool        _isComplete;

	bool        _hasCode;
	bool        _hasData;
	bool        _hasJob;
	bool        _hasResult;

	bool       _isRemoteInstance; 

	Job() {
		
		_hasCode = false;
		_hasData = false;
		_hasJob  = false;
		_hasResult = false;
		
		_isRemoteInstance = false;

		_isComplete = false;
	}
	
	//Returns true if job setup is finalized... 
	bool addResource(Resource& resource) {
		
		std::cout << "Job: Adding Resource!" << std::endl;

		//yup... this is wicked gross.. TODO: Implement working generic prereq system.
		if(resource.type == RESOURCE_TYPE_RESULT) {
			
			//Am I a remote job instance.
			if(_isRemoteInstance) {

				m_result = resource;
				
				_hasResult = true;

			} else {

				m_results.push_back(resource);
			}

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
	
	bool execute() {
	
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

		if(dllHandle == NULL) {
			
			std::cout << "Job: failed to load code... Error: " << GetLastError() << std::endl;
			_hasCode = false;

			return false;
		}

		FUNC func;

		func = (FUNC) GetProcAddress(dllHandle, std::string(infoRes.info.jobName).c_str());
		
		if(func == NULL) {
			
			std::cout << "Job: function retrieval failed .. Error: " << GetLastError() << std::endl;
			_hasCode = false;

			return false;
		}

		std::cout << "****************************RUNNING JOB!!!!**********************" << std::endl;
		
		func(dataRes.buff.getBase(), dataRes.buff.getSize(), m_result.buff);

		std::cout << "*****************************************************************" << std::endl;

		_isComplete = true;
		
		FreeLibrary(dllHandle);
		

		return true;
	}
	
	bool isComplete() {
		
		return _isComplete;
	}

	bool isRunnable() {

		return _hasCode && _hasData && _hasJob && _hasResult;
	}
	
	bool isLocal() {

		return _isRemoteInstance;
	}

	Resource& getResult() {
		
		std::cout << "Job: Getting result!" << std::endl;
		

		m_result.destManager = "net_manager";

		return m_result;
	}

	std::vector<Resource>& getResults() {
		
		return m_results;	
	}
};


