#ifndef _CLIENTS_H_
#define _CLIENTS_H_

#include"Client.h"
#include"Channel.h"
#include<vector>

using namespace std;

class Channel;
class Client;

class Clients
{
	map<string, Client> clients;
	string myClientId;
	Client *host;
	Client varme;
	Channel *m_channel;

public:
	void setMyClientId(string clientId);
	void reset();
	void add(list <Client> clientList);
	void add(Client client);
	void remove(Client client);
	int size();
	Clients();
	Clients(Channel *channel);
	Client get(string clientId);
	Client *getHost() ;//need better way to find host
	Client me(void);
	bool isMe(Client client);
	list<Client> lists();
	Channel *getChannel();
};

#endif
