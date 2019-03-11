#include "Manager.h"
#include "Job.h"

class JobManager : public Manager<JobManager> {
private:
	std::map<int, Job> m_currentIncomingJobs;
    std::map<int, Job> m_currentOutgoingJobs;
public:

	JobManager(IDispatcher& dispatcher) : Manager(dispatcher, "job_manager") {

    }

    void execute() {

        std::vector<Resource> resources;

        getResources(5, resources);
        
        for(int i = 0; i < resources.size(); i++) {
            
            std::cout << "JobManager: new resource received: " << resources[i].type << std::endl;

            auto itr = m_currentIncomingJobs.find(resources[i].jobID);

            if(itr == m_currentIncomingJobs.end()) {
                
                std::cout << "JobManager: Creating new Job: " << resources[i].jobID << std::endl;

                Job job;

                m_currentIncomingJobs.insert({resources[i].jobID, job});
            }

            std::cout << "JobManager: Adding resource to job: " << resources[i].jobID << std::endl;

            Job& job = m_currentIncomingJobs.at(resources[i].jobID);

            job.addResource(resources[i]); 
            
        }

        for (auto it = m_currentIncomingJobs.begin(); it != m_currentIncomingJobs.end(); it++ ) {
            
            //std::cout << "HAS JOB: " << it->second._hasJob << std::endl;
            //std::cout << "HAS DATA: " << it->second._hasData << std::endl;
            //std::cout << "HAS CODE: " << it->second._hasCode << std::endl;

            if(!it->second.isComplete()) {

                it->second.execute();

                //putResources(it->second.getResults());
            }
        }

        for (auto it = m_currentOutgoingJobs.begin(); it != m_currentOutgoingJobs.end(); it++ ) {
            
            //In this case, the job is complete if all prereqs have been satisfied.
            if(it->second.isRunnable()) {

                //std::cout << "JobManager: Job Complete: " << std::endl;
            }
        }

    }

    

    void createJob(std::string codeFn, std::string dataFn, std::string jobName, std::string nodeName) {

        int jobID = rand();

        std::cout << "Creating job: " << jobID << std::endl;

        Job newJob;
        JobInfo info;

        strcpy(info.jobName, jobName.c_str());
 
        Resource code_resource;
        Resource data_resource;
        Resource job_resource;

        job_resource.type          = RESOURCE_TYPE_JOB;
        job_resource.jobID         = jobID;
        job_resource.info          = info;
        job_resource.destManager   = "net_manager";

        strcpy(job_resource.target, nodeName.c_str());
        strcpy(data_resource.target, nodeName.c_str());
        strcpy(code_resource.target, nodeName.c_str());

        Encoder::run(codeFn, code_resource.buff);

        code_resource.type          = RESOURCE_TYPE_CODE;
        code_resource.jobID         = jobID;
        code_resource.codeFn        = std::to_string(jobID) + std::string(".dll");
        code_resource.destManager   = "net_manager";

        data_resource.type          = RESOURCE_TYPE_DATA;
        data_resource.jobID         = jobID;
        data_resource.destManager   = "net_manager";

        Encoder::run(dataFn, data_resource.buff);

        m_currentOutgoingJobs.insert({jobID, newJob});

        std::vector<Resource> temp;
        
        temp.push_back(job_resource);
        temp.push_back(data_resource);
        temp.push_back(code_resource);


        putResources(temp);

        std::cout << "JobManager: Finished creating job" << std::endl;
    }
	
};