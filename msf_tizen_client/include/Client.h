#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include<iostream>
#include <string.h>
#include<map>
//#include"Channel.h"

using namespace std;

class Channel;

class Client 
{
	static string ID_KEY;
	static string IS_HOST_KEY;
	static string CONNECT_TIME_KEY;
	static string ATTRIBUTES_KEY;

	string id;
	bool host;
	long long connectTime;
	map<string, string> attributes;
	Channel *m_channel;

public:
	Client();
	void create(Client &client);
	void create(Channel *channel, string, long long, /*map<string, string> data,*/bool, map<string, string>);
	bool isHost();
	const char *getId();
	long long getConnectTime();
	map<string, string> getAttributesKey();
	Channel *getChannel();
};

#endif
