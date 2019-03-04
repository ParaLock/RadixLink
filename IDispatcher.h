#pragma once

#include <vector>
#include <string>

class IManager;
struct Resource;

class IDispatcher {
private:
public:	

	virtual void dispatch(std::vector<Resource>& resources) = 0;
	virtual void registerManager(std::string name, IManager* manager) = 0;
};