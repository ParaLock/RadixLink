#include "Manager.h"
#include "Job.h"

class JobManager : public Manager<JobManager> {
private:
	std::map<int, Job> m_currentJobs;
public:

	JobManager(IDispatcher& dispatcher) : Manager(dispatcher) {

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

                m_currentJobs.insert({resources[i].jobID, std::move(newJob)});

            } else {
                
                for (auto it = m_currentJobs.begin(); it != m_currentJobs.end(); it++ ) {
                    std::cout << "JobManager: Current Jobs: " << it->first << std::endl;
                }

                Job& job = m_currentJobs.at(resources[i].jobID);
                job.addResource(resources[i]);
            }

        }

        for (auto it = m_currentJobs.begin(); it != m_currentJobs.end(); it++ ) {
            
            if(it->second.isRunnable()) {

                std::cout << "JobManager: Running Job" << std::endl;

                it->second.execute();
            }
        }

    }
	
};