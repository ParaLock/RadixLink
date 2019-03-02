
constexpr unsigned int JOB_NAME_MAX_LENGTH = 20;
constexpr unsigned int JOB_FILE_MAX_LENGTH = 20;

struct JobInfo {
	
	char jobName[JOB_NAME_MAX_LENGTH + 1];
};