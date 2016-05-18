#include "Search.h"
#include "Error.h"
#include "Service.h"
#include "Debug.h"
#include "mDNSSearchProvider.h"
#include "MSFDSearchProvider.h"
#include "Result.h"

int Search::SERVICE_CHECK_TIMEOUT = 5000;
bool Search::clearProviders;
pthread_t Search::threads[NUM_OF_THREADS];
mDNSSearchProvider Search::provider1;
MSFDSearchProvider Search::provider2;
Search *Search::instance = NULL;
bool Search::starting = false;
bool Search::stopping = false;
int Search::search_ref_count = 0;
list<Search*> Search::search_list;
list<Service> Search::services;
int Search::onStartNotified;

int Search::numRunning;

/*
   if (--onStartNotified==0)
   {
   if (onStartListener!=NULL)
   onStartListener->onStart();
   }*/

void Search::onStart()
{
	st_onStart();
}

void Search::onStop()
{
	st_onStop();
}

void Search::onFound(Service service)
{
	st_onFound(service);
}

void Search::onLost(Service service)
{
	st_onLost(service);
}

void  Search :: st_onStart()
{
	dlog_print(DLOG_INFO, "MSF", "Search::onStart()");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

	// ....... ==0 ??
	if (onStartNotified > 0) {
		onStartNotified = 0;
		starting = false; //not sure why ankit
		list<Search*>::iterator itr;
		for (itr = search_list.begin(); itr != search_list.end(); itr++) {
			if ((*itr)->searchListener != NULL &&  (*itr)->searching_now) {
				dlog_print(DLOG_INFO, "MSF", "call onStart");
				(*itr)->searchListener->onStart();
			}
		}
	}
}

void  Search::st_onStop()
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	list<Search*>::iterator itr;
	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onStop();
		}
	}
}

void  Search::st_onFound(Service service)
{
	dlog_print(DLOG_INFO, "MSF", "Search::onFound()");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

	addService(service);

	list<Search*>::iterator itr;

	dlog_print(DLOG_INFO, "MSF", "Search::onFound() list size = %d", search_list.size());

	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onFound(service);
		}
	}
}

void  Search::st_onLost(Service service)
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	//validateService(service);

	list<Search*>::iterator itr;

	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onLost(service);
		}
	}
}

void  SearchListener :: onStart()
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

void  SearchListener :: onStop()
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

void  SearchListener :: onFound(Service service)
{
	//addService(service);
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

void SearchListener :: onLost(Service service)
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

/*void Search::setOnStartListener(OnStartListener *obj){
  cout<<obj;
  onstartlistener=obj;
  }

  void Search::setOnStopListener(OnStopListener obj){
  onstoplistener=obj;
  }

  void Search::setOnServiceFoundListener(OnServiceFoundListener obj){
  onservicefoundlistener=obj;
  }

  void Search::setOnServiceLostListener(OnServiceLostListener obj){
  onservicelostlistener=obj;
  }*/

/*
Search *Search::getInstance()
{
	if (instance == NULL)
		instance = new Search();

	return instance;
}
*/

Search::Search()
{
	searching_now = false;
	searchListener = NULL;

	search_list.push_back(this);
	printf("Search::Search()\n");
	fflush(stdout);
}

Search::~Search()
{
	search_list.remove(this);
	printf("Search::~Search()\n");
	fflush(stdout);

	if (instance) {
		//delete instance;
		//instance = NULL;
	}

	if (searchListener) {
		//delete searchListener;
		//searchListener = NULL;
	}
}

void Search::setSearchListener(SearchListener *obj)
{
	dlog_print(DLOG_INFO, "MSF", "setSearchListener");
	searchListener = obj;
}

void Search::releaseSearchListener()
{
	if (searchListener) {
		searchListener = NULL;
	}
}

/*
bool Search::isSearching()
{
	std::list<SearchProvider>::iterator iterator;
	for (iterator = providers.begin(); iterator != providers.end(); ++iterator) {
		if (iterator->isSearching()) {
			return true;
		}
	}
	return starting||stopping;
}
*/

Service Search::getServiceById(string id)
{
	std::list<Service>::iterator iterator;
	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if ((iterator->getId()) == id) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

bool Search::start()
{
	if (searching_now == false) {
		if (search_ref_count == 0) {
			search_ref_count++;

			searching_now = true;

			dlog_print(DLOG_INFO, "MSF", "Search::start()");

			//if (isSearching()) {
			//	return false;
			//}

			startDiscovery();
		} else {
			search_ref_count++;

			searching_now = true;

			if (searchListener != NULL) {
				dlog_print(DLOG_INFO, "MSF", "call onStart");
				searchListener->onStart();
			}
		}
	}

	return true;
}

bool Search::stop()
{
	if (searching_now == true) {
		search_ref_count--;
		searching_now = false;

		if (search_ref_count == 0) {
			dlog_print(DLOG_INFO, "MSF", "search stop 1");
			stopDiscovery();
		} else {
			if (searchListener != NULL)
				searchListener->onStop();
		}
	}

	return true;
}

list<Service> Search::getServices()
{
	return SearchProvider::getServices();
}

void Search::addProvider(SearchProvider provider)
{
	providers.push_back(provider);
	provider.setSearchListener(this);
}

/*
bool Search::removeProvider(SearchProvider provider)
{
	if (!provider.isSearching())
		return remove(&providers,provider);
	else
		removedProviders.push_back(provider);
	return false;
}
*/

bool Search::isEqualto(SearchProvider s1, SearchProvider s2)
{
	list<Service> services1 = s1.getServices();
	list<Service> services2 = s1.getServices();
	int len1 = services1.size();
	int len2 = services2.size();
	if (len1 == len2)
		return false;

	std::list<Service>::iterator iter1;
	std::list<Service>::iterator iter2;
	for(iter1 = services1.begin(); iter1 != services1.end(); ++iter1) {
		if (services2.size() != 0) {
			iter2 = services2.begin();
			while(iter2 != services2.end()) {
				if (((*iter2).getId()) == ((*iter1).getId())) {
					services2.erase(iter2++);
					break;
				} else {
					iter2++;
				}
			}
		}
	}
	if (services2.size() == 0)
		return true;
	else
		return false;
}

bool Search::remove(list<SearchProvider> *providers, SearchProvider provider)
{
	std::list<SearchProvider>::iterator it = providers->begin();
	while(it != providers->end()) {
		bool match = isEqualto(*it, provider);
		if (match) {
			providers->erase(it);
			return true;
		}
	}
	return false;
}

/*
void Search::processRemovedProviders()
{
	if (removedProviders.size()) {
		std::list<SearchProvider>::iterator iter;
		for(iter = removedProviders.begin(); iter != removedProviders.end(); ++iter) {
			if ((!((*iter).isSearching())) &&(remove(&providers,*iter)))
				remove(&removedProviders,*iter);
		}
	}
}

void Search::removeAllProviders()
{
	clearProviders = false;
	if (!isSearching()) {
		providers.clear();
	}
	else {
		clearProviders = true;
	}
}
*/

void *Search::pt_startMDNS(void *arg)
{
	//provider1.start();
	return NULL;
}

void *Search::pt_startMSFD(void *arg)
{
	provider2.start();
	return NULL;
}

void Search::startDiscovery()
{
	dlog_print(DLOG_INFO, "MSF", "Search::startDiscovery()");
	starting = true;
	// If there are no search providers, then default to using MDNS.
	if (providers.empty()) {
		//providers.push_back(MDNSSearchProvider::create(this)); //CODE NEED TO REMOVE
		providers.push_back(MSFDSearchProvider::create(this));
	}

	services.clear();

	onStartNotified = numRunning = providers.size();

	int ret = -1;

	dlog_print(DLOG_INFO, "MSF", "try MDNS thread creat");
	provider1.start();
//	ret = pthread_create(&threads[MDNS_THREAD_NUMBER],NULL,pt_startMDNS,NULL);
	dlog_print(DLOG_INFO, "MSF", "MDNS thread created");

	if (ret == -1)
		cout << "Fail to create MDNS search provider\n";

	dlog_print(DLOG_INFO, "MSF", "try MSFD thread creat");
	ret = pthread_create(&threads[MSFD_THREAD_NUMBER], NULL, pt_startMSFD, NULL);
	dlog_print(DLOG_INFO, "MSF", "MSFD thread created");

	if (ret == -1)
		cout << "Fail to create MDNS search provider\n";

	onStart();
}

void Search::stopDiscovery()
{
	MSF_DBG("\n[MSF API:]Function Name[%s] got called\n", __FUNCTION__);
	stopping = true;
	bool stop = false;
	stop = provider1.stop();
	stop = provider2.stop();

	if (stop)
		onStop();

	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

	pthread_cancel(threads[1]);
	pthread_join(threads[1], NULL);

	dlog_print(DLOG_INFO, "MSF", "MDNS MSFD thread joined");
	dlog_print(DLOG_INFO, "MSF", "Search::stop discovery end");
}

bool Search::addService(Service service)
{
	dlog_print(DLOG_INFO, "MSF", "Search::addService()");
	bool match = false;
	for (std::list<Service>::iterator it = services.begin(); it != services.end(); it++) {
		if ((*it).getId() == service.getId()) {
			match = true;
			break;
		}
	}
	if (!match)
		services.push_back(service);
	return true;
}

bool Search::removeService(Service service)
{
	for (std::list<Service>::iterator it = services.begin(); it != services.end(); it++) {
		if ((*it).getId() == service.getId()) {
			services.erase(it);
			break;
		}
	}
	return true;
}

void Search::removeAndNotify(Service service)
{
	if (removeService(service)) {
		//		if (onServiceLostListener != NULL) {
		//		searchListener->onLost(service);
		//		}
	}
}

void Search::validateService(Service service)
{
	/*
	class ResultSearchServiceCallback : public Result_Base
	{
		public:
			void onSuccess(Service abc)
			{
				MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
			}	

			void onError(Error)
			{
				MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
			}
	};

	Result_Base *rService;

	ResultSearchServiceCallback *r1Service = new ResultSearchServiceCallback();

	rService = r1Service;

	Service::getByURI(service.getUri(), SERVICE_CHECK_TIMEOUT, rService);
	delete r1Service;
	rService = r1Service = NULL;
	*/
	/*
	   @Override
	   public void onSuccess(Service result) {
	// We can contact the service, so keep it in the master 
	// list.
	}

	@Override
	public void onError(Error error) {

	// If we cannot validate the existence of the service, 
	// then we will remove it from the master list.
	removeAndNotify(service);

	for (SearchProvider provider:providers) {
	provider.removeService(service);
	}
	}
	});*/
}

