#pragma once


struct Resource;

class IManager {
private:
public:

    virtual std::string getName() = 0;
    virtual void        addResource(Resource& resource, std::string name) = 0;
    virtual void        execute() = 0;

};