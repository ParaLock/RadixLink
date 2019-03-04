#pragma once


struct Resource;

class IManager {
private:
public:

    virtual std::string getName() = 0;
    virtual void        addResource(Resource& resource) = 0;

};