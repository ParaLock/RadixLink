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


            if(resources[i].type == RESOURCE_TYPE_JOB) {
                
                std::cout << "JobManager: Creating Job" << std::endl;

                Job newJob;

                    
                int count = 0;
                while(resources[i].info.preReqs[count] != -1) {

                    std::cout << "JobManager: Adding Job PreReq!" << std::endl;

                    newJob.addPreReq(resources[i].info.preReqs[count]);

                    count++;
                }

                m_currentIncomingJobs.insert({resources[i].jobID, std::move(newJob)});
                
            }

            Job& job = m_currentIncomingJobs.at(resources[i].jobID);
            job.addResource(resources[i]); 
            
        }

        for (auto it = m_currentIncomingJobs.begin(); it != m_currentIncomingJobs.end(); it++ ) {
            
            if(it->second.isRunnable()) {

                std::cout << "JobManager: Running Job" << std::endl;

                it->second.execute();

                m_dispatcher.dispatch(it->second.getResults());
            }
        }

        for (auto it = m_currentOutgoingJobs.begin(); it != m_currentOutgoingJobs.end(); it++ ) {
            
            //In this case, the job is complete if all prereqs have been satisfied.
            if(it->second.isRunnable()) {

                std::cout << "JobManager: Job Complete: " << std::endl;
            }
        }

    }

    void createJob(std::string codeFn, std::string dataFn, std::string jobName, std::string nodeName) {

        int jobID = rand();

        Job newJob;
        JobInfo info;

        strcpy(info.jobName, jobName.c_str());
 
        Resource code_resource;
        Resource data_resource;
        Resource job_resource;

        job_resource.type          = RESOURCE_TYPE_JOB;
        job_resource.jobID         = jobID;
        job_resource.info          = info;

        strcpy(job_resource.target, nodeName.c_str());
        strcpy(data_resource.target, nodeName.c_str());
        strcpy(code_resource.target, nodeName.c_str());


        Encoder::run(codeFn, code_resource.buff);

        code_resource.type          = RESOURCE_TYPE_CODE;
        code_resource.jobID         = jobID;
        code_resource.codeFn        = std::to_string(b.jobID) + std::string(".dll");

        data_resource.type          = RESOURCE_TYPE_DATA;
        data_resource.jobID         = jobID;


    }
	
};