#include "IDispatcher.h"
#include "IManager.h"

class Dispatcher {
private:
	std::map<std::string, std::function<void(Resource&)>> m_managers;
public:
	
	Dispatcher() {
		
		
		
	}

	void registerManager(std::string name, IManager& manager) {

		m_managers.insert({name, [manager](Resource& resource) {
			manager.addResource(resource);
		}
		}});
	}
	
	void dispatch(std::vector<Resource>& resources) {
		
		for(int i = 0; i < resources.size(); i++) {

			m_managers.at(resources[i].destManager)(resources[i]);
		}
		
	}

};