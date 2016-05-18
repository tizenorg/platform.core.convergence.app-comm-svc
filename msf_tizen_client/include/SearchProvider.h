#ifndef _MSF_SEARCHPROVIDER_H
#define _MSF_SEARCHPROVIDER_H

#include<iostream>
#include<list>
#include<string>
//#include "Service.h"
//#include "Search.h"

using namespace std;

class Service;
class Search;

class SearchProvider
{
private:
	Search *searchListener;
	static list<Service> services;

public:
	SearchProvider();
	SearchProvider(Search *);
	virtual void start() {}
	virtual bool stop() { return true; }
	static list<Service> getServices();
	bool isSearching();
	void setSearchListener(Search *obj);

protected:
	bool searching;
	void setServices(list<Service>);
	void addService(Service service);
	void removeService(Service service);
	void removeServiceAndNotify(Service service);
	void clearServices();
	Service getServiceById(string id);
};

#endif
