#include"Clients.h"

Clients::Clients()
{
	host = NULL;
	m_channel = NULL;
}

Clients::Clients(Channel *channel)
{
	host = NULL;
	m_channel = channel;
}

Client Clients::get(string clientId)
{
	Client cl = clients.find(clientId)->second;
	return cl;
}

Client* Clients::getHost()
{
	if (host == NULL) {
		std::vector<Client> clients;
		for (std::vector<Client>::iterator it = clients.begin() ; it != clients.end(); ++it) {
			if ((*it).isHost()) {
				host = new Client();
				host->create(*it);
			}
		}
	}
	return host;
}

Client Clients::me(void)
{
	if (myClientId != "") {
		Client client = get(myClientId);
		if ((&client != NULL) && !(client.getId() == varme.getId())) {
			varme = client;
		}
	}
	return varme;
}

bool Clients::isMe(Client client)
{
	return (client.getId()== myClientId);
}

/*int Clients::ssize()
  {
  return sizeof(clients);
  }*/

list<Client> Clients::lists()
{
   list<Client> client_list;
   std::map<string, Client>::iterator clients_itr;

   for (clients_itr = clients.begin(); clients_itr != clients.end(); clients_itr++) {
	   client_list.push_back(clients_itr->second);
   }

   return client_list;
}

void Clients::setMyClientId(string clientId)
{
	myClientId = clientId;
	Client client = get(myClientId);
	varme = client;
}

void Clients::reset()
{
	clients.clear();
	if (host != NULL)
		delete host;
	host = NULL;
	myClientId = "";
}

void Clients::add(list <Client> clientList)
{
	std::list<Client>::const_iterator iterator;
	for (iterator = clientList.begin(); iterator != clientList.end(); ++iterator) {
		add(*iterator);
	}
}

void Clients::add(Client client)
{
	clients.insert({string(client.getId()), client});
}

int Clients::size()
{
	return clients.size();
}

void Clients::remove(Client client)
{
	clients.erase(client.getId());
}

Channel* Clients::getChannel()
{
	return m_channel;
}
