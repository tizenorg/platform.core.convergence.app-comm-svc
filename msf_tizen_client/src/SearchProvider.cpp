#include"Service.h"
#include "Search.h"
#include"SearchProvider.h"
#include "Debug.h"

list<Service> SearchProvider::services;

SearchProvider::SearchProvider()
{
	searching = false;
}

SearchProvider::SearchProvider(Search *searchListen)
{
	searchListener = searchListen;
	searching = false;
}

list<Service> SearchProvider::getServices()
{
	return services;
}

void SearchProvider::setServices(list<Service> listofservices)
{
	clearServices();
	if (listofservices.size() != 0) {
		std::list<Service>::const_iterator iterator;
		for (iterator = listofservices.begin(); iterator != listofservices.end(); ++iterator) {
			services.push_back(*iterator);
		}
	}
}

void SearchProvider::setSearchListener(Search *obj)
{
	searchListener = obj;
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

void SearchProvider::addService(Service service)
{
	dlog_print(DLOG_INFO, "MSF", "SearchProvider::addService()");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	bool found = false;

	std::list<Service>::iterator iterator;

	bool match = false;

	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if (iterator->getId() == service.getId()) {
			match = true;
			break;
		}
	}

	if (!match) {
		services.push_back(service);
		found = true;
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		if (found) {
			Search::st_onFound(service);
		}
	} else {
	}
}

void SearchProvider::removeService(Service service)
{
	if (&service == NULL) {
		return;
	}

	std::list<Service>::iterator it = services.begin();
	while (it != services.end()) {
		if (((*it).getId()) == (service.getId())) {
			services.erase(it++);
		} else {
			it++;
		}
	}
}

void SearchProvider::removeServiceAndNotify(Service service)
{
	removeService(service);
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	Search::st_onLost(service);
}

void SearchProvider::clearServices()
{
	services.clear();
}

Service SearchProvider::getServiceById(string id)
{
	//	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n",__FUNCTION__ ,__LINE__,__FILE__);
	std::list<Service>::iterator iterator;
	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if (((*iterator).getId()) == id) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

bool SearchProvider::isSearching()
{
	if (this->searching)
		return true;
	else
		return false;
}
