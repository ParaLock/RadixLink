#pragma once

#include "Manager.h"
#include "Job.h"
#include "DllLoader.h"
#include "Utils.h"
#include "NetworkManager.h"

class JobManager : public Manager<JobManager> {
private:
	std::map<int, Job> m_currentIncomingJobs;
    std::map<int, Job> m_currentOutgoingJobs;

    NetworkManager& m_netMan;

public:

	JobManager(IDispatcher& dispatcher, TaskQueue& taskQueue, NetworkManager& netMan, StateRegistry& reg) 
        : Manager(dispatcher, taskQueue, reg, "job_manager", false), m_netMan(netMan)
          
    {
        m_stateReg.addState<std::string>("running_job", "none");
    }

    void execute() {

        std::vector<Resource> resources;

        getResources(5, resources, "primary");
        
        for(int i = 0; i < resources.size(); i++) {
            
            std::cout << "JobManager: new resource received: " << resources[i].type << std::endl;

            if(resources[i].type == RESOURCE_TYPE_RESULT) {

                auto itr = m_currentOutgoingJobs.find(resources[i].jobID);

                if(itr != m_currentOutgoingJobs.end()) {
                    
                    resources[i].destManager = "net_manager";
                    itr->second.addResource(resources[i]);

                    continue;
                }
            }

            auto itr = m_currentIncomingJobs.find(resources[i].jobID);

            if(itr == m_currentIncomingJobs.end()) {
                
                std::cout << "JobManager: Creating new Job: " << resources[i].jobID << std::endl;

                Job job;
                job._isRemoteInstance = true;

                m_currentIncomingJobs.insert({resources[i].jobID, job});
            }

            std::cout << "JobManager: Adding resource to job: " << resources[i].jobID << std::endl;

            Job& job = m_currentIncomingJobs.at(resources[i].jobID);

            job.addResource(resources[i]); 
            
        }

        for (auto it = m_currentIncomingJobs.begin(); it != m_currentIncomingJobs.end(); it++ ) {
            
            if(it->second.isRunnable() && !it->second.isComplete()) {

                it->second.execute();

                std::vector<Resource> temp;
                temp.push_back(it->second.getResult());

                this->putResources(temp, "primary");
            }
        }
    }

    void printJobResults(int jobID) {
        
        std::cout << "Printing results of job " << jobID << " -> "<< std::endl;

        Job& job = m_currentOutgoingJobs.at(jobID);

        std::vector<Resource>& results = job.getResults();

        for(int i = 0; i < results.size(); i++) {
            results[i].buff.print();
        }
    }

    bool writeJobResultToDisk(unsigned int jobID) {

        std::cout << "Writing result of job " << jobID << " to disk! "<< std::endl;

        if(m_currentOutgoingJobs.find(jobID) == m_currentOutgoingJobs.end()) {

            std::cout << "JobManager: unknown job: " << jobID << std::endl;

            return false;
        }

        Job& job = m_currentOutgoingJobs.at(jobID);

        std::ofstream binFile(std::to_string(jobID) + "_final_result", std::ios::out | std::ios::binary);
        
        if (binFile.is_open()) {
            
            binFile.write(job.m_result.buff.getBase(), job.m_result.buff.getSize());

            return true;

        } else {

            std::cout << "JobManager: Failed to write result to disk!" << std::endl;

            return false;
        }

    }

    bool createJob(std::string codeFn, std::string dataFn, std::string jobName, std::vector<std::string> availableNodes, int& jobIDOut) {

        int jobID = rand();

        jobIDOut = jobID;

        std::cout << "Creating job: " << jobID << std::endl;

        Job newJob;
        newJob.id = jobID;
        newJob._isRemoteInstance = false;
        newJob.m_codeFn = codeFn;

        std::vector<Buffer> segments;

        std::cout << "JobManager: segmenting data..." << std::endl;

        DllLoader dllLoader;

        if(!dllLoader.load(codeFn)) {

            std::cout << "JobManager: Failed to load segmentor! in " << codeFn << std::endl;

            return false; 
        } 

        Buffer jobData;

        if(!Encoder::run(dataFn, jobData)) {

            std::cout << "JobManager: Failed to open job data file: " << std::endl;

            return false;
        }

        auto& activeNodes = m_netMan.getActiveNodes();

        if(!dllLoader.call<int, Buffer&, std::vector<Buffer>&>("segmentData", activeNodes.size(), jobData, segments)) {

            std::cout << "JobManager: Failed to run job segmentor routine!" << std::endl;

            return false;
        }

        newJob.setNumSegments(segments.size());

        std::cout << "JobManager: num segments: " << segments.size() << std::endl;
        std::cout << "JobManager: num available nodes" << availableNodes.size() << std::endl;


        if(availableNodes.size() < segments.size()) {
            
            std::cout << "JobManager: Not enough nodes to process entire job..." << std::endl;

            return false;
        }

        m_currentOutgoingJobs.insert({jobID, newJob});

        unsigned int order = 0;

        for(int i = 0; i < segments.size(); i++) {

            JobInfo info;

            strcpy(info.jobName, jobName.c_str());
    
            Resource code_resource;
            Resource data_resource;
            Resource job_resource;
            Resource result_resource;

            job_resource.type          = RESOURCE_TYPE_JOB;
            job_resource.jobID         = jobID;
            job_resource.info          = info;
            job_resource.destManager   = "net_manager";

            strcpy(job_resource.target, availableNodes[i].c_str());
            strcpy(data_resource.target, availableNodes[i].c_str());
            strcpy(code_resource.target, availableNodes[i].c_str());
            strcpy(result_resource.target, availableNodes[i].c_str());

            Encoder::run(codeFn, code_resource.buff);

            code_resource.type          = RESOURCE_TYPE_CODE;
            code_resource.jobID         = jobID;
            code_resource.codeFn        = std::to_string(jobID) + std::string(".dll");
            code_resource.destManager   = "net_manager";

            data_resource.type          = RESOURCE_TYPE_DATA;
            data_resource.jobID         = jobID;
            data_resource.destManager   = "net_manager";
            data_resource.order         = order;

            result_resource.type        = RESOURCE_TYPE_RESULT;
            result_resource.jobID = jobID;
            result_resource.destManager = "net_manager";

            data_resource.buff = segments[i];

            std::vector<Resource> temp;
            
            temp.push_back(job_resource);
            temp.push_back(data_resource);
            temp.push_back(code_resource);
            temp.push_back(result_resource);

            putResources(temp, "primary");

            std::cout << "JobManager: Job segment created for " << availableNodes[i] << std::endl;

            order++;
        }

        std::cout << "JobManager: Finished creating job" << std::endl;

        return true;

    }
	
};