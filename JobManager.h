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

                m_currentIncomingJobs.insert({resources[i].jobID, std::move(Job())});
            }

            Job& job = m_currentIncomingJobs.at(resources[i].jobID);

            if(resources[i].type == RESOURCE_TYPE_RESULT) {

                m_currentOutgoingJobs.at(resources[i].jobID).addResource(resources[i]);

                continue;
            }

            if(resources[i].type == RESOURCE_TYPE_JOB) {
                
                std::cout << "JobManager: Creating Job" << std::endl;

                int count = 0;
                while(resources[i].info.preReqs[count] != -1) {

                    std::cout << "JobManager: Adding Job PreReq!" << std::endl;

                    job.addPreReq(resources[i].info.preReqs[count]);

                    count++;
                }
                
            }

            job.addResource(resources[i]); 
            
        }

        for (auto it = m_currentIncomingJobs.begin(); it != m_currentIncomingJobs.end(); it++ ) {
            
            if(it->second.isRunnable() && !it->second.isComplete()) {

                it->second.execute();

                putResources(it->second.getResults());
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

        info.preReqs[0] = RESOURCE_TYPE_JOB;
        info.preReqs[1] = RESOURCE_TYPE_CODE;
        info.preReqs[2] = RESOURCE_TYPE_DATA;

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