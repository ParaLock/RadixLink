#pragma once

#include <algorithm>

#include "windows.h"

#include "Resource.h"
#include "JobInfo.h"
#include "Encoder.h"

#include "DllLoader.h"

struct Job {

	std::map<int, Resource>    m_resources;
	std::vector<Resource>      m_results;

	std::string 			   m_codeFn;
	Resource 				   m_result;
	int						   m_numSegments;
	int id;

	unsigned int               jobType;

	bool        _isComplete;
	bool        _hasCode;
	bool        _hasData;
	bool        _hasJob;
	bool        _hasResult;
	bool 		_hasRun;
	bool       _isRemoteInstance; 

	Job() {
		jobType = -1;
		_hasCode = false;
		_hasData = false;
		_hasJob  = false;
		_hasResult = false;
		_hasRun = false;

		
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

				combineResults();

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
	
	virtual bool execute() = 0;
	virtual bool combineResults() = 0;
	virtual bool segmentData(Buffer& input, std::string codeFn, int activeNodes, std::vector<Buffer>& segments) = 0; 
	
	bool hasRun() {

		return _hasRun;
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

	void setNumSegments(int segs) {
		m_numSegments = segs;
	}

	int getNumSegments() {
		return m_numSegments;
	}

	int getNumResults() {

		return m_results.size();
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


