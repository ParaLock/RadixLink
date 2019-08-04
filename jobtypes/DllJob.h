#include "../Job.h"
#include "../DllLoader.h"
#include "../Resource.h"

class DllJob : public Job {

public:

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
	
	bool execute() {

		Resource& codeRes = m_resources.at(RESOURCE_TYPE_CODE);
		Resource& dataRes = m_resources.at(RESOURCE_TYPE_DATA);
		Resource& infoRes = m_resources.at(RESOURCE_TYPE_JOB);

		std::cout << "Job: filename: " << codeRes.codeFn << std::endl;
		std::cout << "Job: job name: ";

		for (int i = 0; i < 20; i++) {
			std::cout << infoRes.info.jobName[i];
		}

		std::cout << std::endl;

		DllLoader dllLoader;

		if (!dllLoader.load(codeRes.codeFn.c_str())) {

			return false;
		}

		std::cout << "****************************RUNNING JOB!!!!**********************" << std::endl;


		auto getInput = [&dataRes](char*& src, size_t& size) {

			src = dataRes.buff.getBase();
			size = dataRes.buff.getSize();
		};

		auto expandOutput = [this](size_t size) {

			m_result.buff.resize(m_result.buff.getSize() + size);
		};

		auto writeOutput = [this](char* src, size_t size) {

			m_result.buff.write(src, size);
		};
		
		auto getOutput = [this](char*& ptr, size_t& size) {

			ptr = m_result.buff.getBase();
			size = m_result.buff.getSize();
		};


		dllLoader.call<
						std::function<void(char*&, size_t&)>,
                    	std::function<void(size_t)>,
                    	std::function<void(char*, size_t)>,
						std::function<void(char*&, size_t&)>
					  >

			("run", getInput, expandOutput, writeOutput, getOutput);

		std::cout << "*****************************************************************" << std::endl;

		_isComplete = true;
		
		return true;
	}

	bool combineResults() {

		if(!_isRemoteInstance && getNumSegments() == getNumResults()) {
			
			std::cout << "Job:  Combining Results!" << std::endl;

			DllLoader dllLoader;

			if(!dllLoader.load(m_codeFn.c_str())) {

				return false;
			}

			std::vector<Resource*> sortedResults;

			for(int i = 0; i < m_results.size(); i++) {

				sortedResults.push_back(&m_results[i]);
			}

			std::sort(sortedResults.begin(), sortedResults.end(), [](Resource* a, Resource* b) {
				return a->order < b->order;   
			});

			std::vector<Buffer*> temp;

			for(int i = 0; i < sortedResults.size(); i++) {

				temp.push_back(&sortedResults[i]->buff);
			}


			m_result.buff.clear();
			
			auto getSegment = [&temp](char*& segOut, size_t& size, int seg) {
			
				Buffer* buff = temp.at(seg);
				
				size = buff->getSize();
				segOut = buff->getBase();

			};

			auto getOutput = [this](char*& ptr, size_t& size) {

				ptr  = m_result.buff.getBase();
				size = m_result.buff.getSize();
			};

			auto expandOutput = [this](size_t size) {
			
				m_result.buff.resize(m_result.buff.getSize() + size);

			};

			auto writeOutput = [this](char* src, size_t size) {

				m_result.buff.write(src, size);
			};

			dllLoader.call<
							int,
                            std::function<void(char*&, size_t&, int)>,
							std::function<void(char*&, size_t&)>,
                        	std::function<void(size_t)>,
                            std::function<void(char*, size_t)> 
						>
				("combine", temp.size(), getSegment, getOutput, expandOutput, writeOutput);

			_hasRun = true;
			
		}

		return true;
	}


    bool segmentData(Buffer& jobData, std::string codeFn, int activeNodes, std::vector<Buffer>& segments) {


        DllLoader dllLoader;

        if(!dllLoader.load(codeFn)) {

            std::cout << "DllJob: Failed to load segmentor! in " << codeFn << std::endl;

            return false; 
        } 

        auto getInput = [&jobData](char*& src, size_t& size) {

            src = jobData.getBase();
            size = jobData.getSize();
        };

		auto writeSegment = [&segments](char* src, size_t size, int seg) {

            std::cout << "JobManager: writeSegment: seg: " << seg << " size: " << size << " existing segments: " << " pointer: " << static_cast<void*>(src) << segments.size() << std::endl;

			while (segments.size() < seg + 1) {

                std::cout << "JobManager: writeSegment: creating new segment! seg: " << seg << std::endl;

				segments.push_back(Buffer());
			}

			Buffer& buff = segments.at(seg);

			buff.write(src, size);
		};
        
        auto getSegment = [&segments](char*& src, size_t& size, int seg) {

            std::cout << "JobManager: getSegment: seg: " << seg << std::endl;

			while (segments.size() < seg + 1) {
                
                std::cout << "JobManager: getSegment: creating new segment! seg: " << seg << std::endl;

				segments.push_back(Buffer());
			}

			Buffer& buff = segments.at(seg);

            size = buff.getSize();
            src  = buff.getBase();
		};

        auto expandSegment = [&segments](size_t size, int seg) {

			while (segments.size() < seg + 1) {

                std::cout << "JobManager: expandSegment: creating new segment! seg: " << seg << std::endl;

				segments.push_back(Buffer());
			}

			Buffer& buff = segments.at(seg);

			buff.resize(buff.getSize() + size);
		};

        if(!dllLoader.call<
                            int,
                            std::function<void(char*&, size_t&)>,
                            std::function<void(char*&, size_t&, int)>,
                            std::function<void(size_t, int)>,
                            std::function<void(char*, size_t, int)>
                            >(
			"segmentData", activeNodes, getInput, getSegment, expandSegment, writeSegment)) {

            std::cout << "JobManager: Failed to run job segmentor routine!" << std::endl;

            return false;
        }

       m_numSegments = activeNodes;

        std::cout << "JobManager: num segments: " << segments.size() << std::endl;
        std::cout << "JobManager: num available nodes" << activeNodes << std::endl;

        return true;
    }

};