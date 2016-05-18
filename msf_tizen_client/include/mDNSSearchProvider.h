#ifndef MDNSSEARCHPROVIDER_H
#define MDNSSEARCHPROVIDER_H

using namespace std;

#include"SearchProvider.h"
#include <nsd/dns-sd.h>

class mDNSSearchProvider : public SearchProvider
{
private:

public:

	mDNSSearchProvider();
	~mDNSSearchProvider();
	void addService(Service service);
	mDNSSearchProvider(Search *sListener);
	void start();
	bool stop();
	static SearchProvider create();
	static SearchProvider create(Search *);
};

#endif
