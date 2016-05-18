#ifndef _MSF_SEARCH_H
#define _MSF_SEARCH_H

//#include"Service.h"
#include <list>
#include"SearchProvider.h"
#include <list>
#include <pthread.h>
#include <string>
#include"mDNSSearchProvider.h"
//#include"MSFDSearchProvider.h"

#define NUM_OF_THREADS 2
#define MDNS_THREAD_NUMBER 0
#define MSFD_THREAD_NUMBER 1

using namespace std;

class Service;
class MSFDSearchProvider;
class mDNSSearchProvider;

class OnStartListener
{
public:
	virtual void onStart() { }
};

class OnStopListener
{
public:
	virtual void onStop() { }
};

class OnServiceFoundListener
{
public:
	virtual void onFound(Service) = 0;
};

class OnServiceLostListener
{
public:
	virtual void onLost(Service) = 0;
};

class SearchListener:public OnStartListener, public OnStopListener, public OnServiceFoundListener, public OnServiceLostListener
{
public:
	virtual ~SearchListener(){}
	virtual void onStart();
	virtual void onStop();
	virtual void onFound(Service);
	virtual void onLost(Service);
};

class Search:public SearchListener
{
private :
	static int SERVICE_CHECK_TIMEOUT;
	static Search *instance;
	list<SearchProvider> providers ;
	list<SearchProvider> removedProviders ;
	static list<Service> services;

	static int numRunning;
	SearchListener *searchListener;
	static bool clearProviders;
	static bool starting;
	static int search_ref_count;
	static bool stopping;
	bool searching_now;
	static mDNSSearchProvider provider1;
	static MSFDSearchProvider provider2;
	//OnStartListener *onStartListener;
	//OnStopListener *onStopListener;
	//OnServiceFoundListener *onServiceFoundListener;
	//OnServiceLostListener *onServiceLostListener;

public:
	static pthread_t threads[NUM_OF_THREADS];
	static int onStartNotified;
	//static Search* getInstance();
	Search();
	~Search();
	//bool isSearching();
	bool start();
	bool stop();

	static list<Search*> search_list;
	void onStart();
	void onStop();
	void onFound(Service);
	void onLost(Service);
	static void st_onStart();
	static void st_onStop();
	static void st_onFound(Service);
	static void st_onLost(Service);

	bool isEqualto(SearchProvider, SearchProvider);
	list<Service> getServices();
	bool remove(list<SearchProvider> *, SearchProvider);
	void addProvider(SearchProvider);
	//bool removeProvider(SearchProvider);
	//void processRemovedProviders();
	//void removeAllProviders();
	void startDiscovery();
	void stopDiscovery();
	static bool addService(Service);
	bool removeService(Service);
	void removeAndNotify(Service);
	void validateService(Service);
	//void setOnStartListener(OnStartListener *);
	//void setOnStopListener(OnStopListener);
	//void setOnServiceFoundListener(OnServiceFoundListener);
	//void setOnServiceLostListener(OnServiceLostListener);
	Service getServiceById(string id);

	void setSearchListener(SearchListener*);
	void releaseSearchListener();
	static void *pt_startMDNS(void *);
	static void *pt_startMSFD(void *);
};

#endif
