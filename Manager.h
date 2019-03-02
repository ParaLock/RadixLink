
#include <thread>

class Manager {
private:
    std::thread executionThread;
public:
    void addResources(std::vector<Resource>& resources) {


    }

    virtual void execute() = 0;

    void start() {

        executionThread = std::thread(execute);
    }

    void stop() {

        executionThread.join();
    }
};