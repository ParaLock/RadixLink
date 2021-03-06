#pragma once

#include "IDispatcher.h"
#include "IManager.h"
#include "Resource.h"

class Dispatcher : public IDispatcher {
private:

	std::map<std::string, IManager*> m_managers;
public:
	
	Dispatcher() {}

	void registerManager(std::string name, IManager* manager) {

		m_managers.insert({name, manager});
	}
	
	void dispatch(std::vector<Resource>& resources, std::string group) {
		
		for(int i = 0; i < resources.size(); i++) {
			
			std::cout << "Dispatcher: Dispatched resource " << resources[i].type << " to " << resources[i].destManager << " group: " << group << std::endl;

			m_managers.at(resources[i].destManager)->addResource(resources[i], group);

			std::cout << "Dispatch: Resource dispatched successfully!" << std::endl;
		}
		
	}

};