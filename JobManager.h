#include "Manager.h"
#include "Job.h"

class JobManager : Manager<JobManager> {
private:
	std::map<int, Job> m_currentJobs;
public:

	JobManager(IDispatcher& dispatcher) : Manager(dispatcher) {

    }

    void execute() {

        auto resources = getResources(5);
        
        for(int i = 0; i < resources.size(); i++) {

            if(resources[i].type == RESOURCE_TYPE_JOB) {

                Job newJob;
                
                int count = 0;
                while(resources[i].senderName[count] != -1) {

                    newJob.addPreReq(resources[i].senderName[count]);

                    count++;
                }

                m_currentJobs[resources[i].jobID] = newJob;
            }

            Job& job = m_currentJobs[resources[i].jobID];
            job.addResource(resources[i]);
        }

        for (auto it = m_currentJobs.begin(); it != m_currentJobs.end(); it++ ) {
            
            if(it->second.isRunnable()) {
                it->second.execute();
            }
        }

    }
	
};