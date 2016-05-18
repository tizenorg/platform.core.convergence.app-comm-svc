#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "MSFDSearchProvider.h"
#include "Service.h"
#include "Debug.h"
#include "Error.h"
#include "Result.h"
#define MSGBUFSIZE 1000
#define MULTICAST_PORT 8001
#define MULTICAST_GROUP "224.0.0.7"

string MSFDSearchProvider::KEY_TYPE_STATE = "type";
string MSFDSearchProvider::KEY_TTL = "ttl";
string MSFDSearchProvider::KEY_SID = "sid";
string MSFDSearchProvider::KEY_DATA = "data";
string MSFDSearchProvider::KEY_VERSION_1 = "v1";
string MSFDSearchProvider::KEY_VERSION_2 = "v2";
string MSFDSearchProvider::KEY_URI = "uri";
string MSFDSearchProvider::TYPE_DISCOVER = "discover";
string MSFDSearchProvider::STATE_UP = "up";
string MSFDSearchProvider::STATE_DOWN = "down";
string MSFDSearchProvider::STATE_ALIVE = "alive";
int MSFDSearchProvider::SERVICE_CHECK_TIMEOUT = 5000;

int MSFDSearchProvider::flag = 0;

MSFDSearchProvider::MSFDSearchProvider()
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	SearchProvider();
}

MSFDSearchProvider::MSFDSearchProvider(Search *sListener):SearchProvider(sListener)
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

SearchProvider MSFDSearchProvider::create()
{
	return (SearchProvider)MSFDSearchProvider();
}

SearchProvider MSFDSearchProvider::create(Search *searchListener)
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	return (SearchProvider)MSFDSearchProvider(searchListener);
}

void MSFDSearchProvider::start()
{
	dlog_print(DLOG_INFO, "MSF", "MSFD start()");
	if (searching) {
		stop();
	}
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	clearServices();

	aliveMap.clear();
	receive = false;
	createMSFD();
}

void MSFDSearchProvider::createMSFD()
{
	struct sockaddr_in addr;
	struct sockaddr_in msf_server_addr;
	int  nbytes;
	unsigned int addrlen;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];
	fd = 0;
	u_int yes = 1;          /*** MODIFICATION TO ORIGINAL */

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		dlog_print(DLOG_ERROR, "MSF", "MSFD socket faile");
	}

	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		dlog_print(DLOG_ERROR, "MSF", "MSFD reusing ADDR failed");
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
	addr.sin_port = htons(MULTICAST_PORT);

	dlog_print(DLOG_INFO, "MSF", "MSFD try bind socket");
	/* bind to receive address */
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		dlog_print(DLOG_ERROR, "MSF", "MSFD bind failed");
	}
	dlog_print(DLOG_INFO, "MSF", "MSFD try bind socket success");

	/* use setsockopt() to request that the kernel join a multicast group */
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		dlog_print(DLOG_ERROR, "MSF", "MSFD setsockopt failed");
	}

	while (1) {
		addrlen = sizeof(msf_server_addr);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 900000;

		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
			perror("setsockopt");
			dlog_print(DLOG_ERROR, "MSF", "MSFD setsockopt(SOL_SOCKET) failed");
		}

			dlog_print(DLOG_INFO, "MSF", "MSFD waiting packet");
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &msf_server_addr, &addrlen)) < 0) {
			//dlog_print(DLOG_INFO, "MSF", "MSFD packet < 0");
			receive = false;
			dlog_print(DLOG_INFO, "MSF", "MSFD packet < 0 1");
			reapServices();
			dlog_print(DLOG_INFO, "MSF", "MSFD packet < 0 2");
		} else {
			msgbuf[nbytes] = '\0';
			dlog_print(DLOG_INFO, "MSF", "MSFD waiting packet = %s", msgbuf);
			receive = true;
			processReceivedMsg(msgbuf, nbytes);
		}
	}
}

void MSFDSearchProvider::reapServices()
{
	dlog_print(DLOG_ERROR, "MSF", "MSFD reapServices");
	long now = time(0);
	map<string, long>::iterator it;
	for(it = aliveMap.begin(); it != aliveMap.end(); ++it) {
		long expires = it->second;
		if (expires < now) {
			dlog_print(DLOG_ERROR, "MSF", "MSFD reapServices remove service");
			Service service = getServiceById(it->first);
			aliveMap.erase(it->first);
			removeServiceAndNotify(service);
		}
	}
}

void MSFDSearchProvider::updateAlive(string id, long ttl)
{
	long now = time(0);
	ttl = ttl/1000;
	long expires = now+ttl;
	aliveMap[id] = expires;
}

void MSFDSearchProvider::processReceivedMsg(char *buf, int buflen)
{
	dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg start");
	if (buf != NULL) {
	}

	while(receive) {
	dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg 1");
		reapServices();
	dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg 2");
		string data;
		data.append(buf, buflen);
		json_parse(data.c_str());

		dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg after parse");

		if (state == TYPE_DISCOVER) {
			dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg state = discover");
			continue;
		}

		if (id.length() > 0) {
			dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg id length > 0");
			Service serv =	getServiceById(id);
			Service *serv1 = &serv;

			if (serv.getId() != "") {
				receive = false;
			}

			if (state == STATE_ALIVE || state == STATE_UP) {
				map<string, long>::iterator i = aliveMap.find(id);
				if (((serv.getId()).length() == 0)) {//&&(i==aliveMap.end()))
					updateAlive(id, ttl);
					static map<string, long>* tempaliveMap = &aliveMap;
					static MSFDSearchProvider *MSFDSearchProvider_pointer = this;
					static string tempid = id;
					static long tempttl = ttl;

					class ResultMSFDServiceCallback : public Result_Base
					{
						public:
							void onSuccess(Service abc)
							{
								MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
								MSFDSearchProvider_pointer->updateAlive(tempid, tempttl);
								//(*tempaliveMap)[tempid]=tempttl;
								MSFDSearchProvider_pointer->addService(abc);
							}

							void onError(Error)
							{
								MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
								tempaliveMap->erase(tempid);
								//aliveMap.erase(id);
							}
					};
					Result_Base *rService = NULL;
					ResultMSFDServiceCallback *r1Service = new ResultMSFDServiceCallback();
					rService = r1Service;
					Service::getByURI(url, SERVICE_CHECK_TIMEOUT, rService);

					// ........ critical bug
					//delete r1Service;
					//r1Service = rService = NULL;
				} else {
					updateAlive(id, ttl);
				}
			} else if ((serv1->getId() != "") && (state == STATE_DOWN)) {
				aliveMap.erase(id);
				removeServiceAndNotify(serv);
			}
		} else {
			receive = false;
		}
	}
	dlog_print(DLOG_INFO, "MSF", "MSFD processReceivedMsg end");
}

void MSFDSearchProvider::foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data)
{
	MSFDSearchProvider *p = static_cast<MSFDSearchProvider*>(user_data);

	if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
		if (!strncmp(key , MSFDSearchProvider::KEY_TTL.c_str(), 2)) {
			p->ttl = json_node_get_int(node);
			dlog_print(DLOG_INFO, "MSF", "MSFD ttl = %d", p->ttl);
		} else if (!strncmp(key , MSFDSearchProvider::KEY_TYPE_STATE.c_str(), 2)) {
			p->state = json_node_get_string(node);
			dlog_print(DLOG_INFO, "MSF", "MSFD state = %s", p->state.c_str());
		} else if (!strncmp(key, MSFDSearchProvider::KEY_SID.c_str(), 4)) {
			p->id = json_node_get_string(node);
			dlog_print(DLOG_INFO, "MSF", "MSFD id = %s", p->id.c_str());
		} else if (!strncmp(key, MSFDSearchProvider::KEY_URI.c_str(), 7)) {
			if (flag == 1) {
				p->url = json_node_get_string(node);
				dlog_print(DLOG_INFO, "MSF", "MSFD url = %s", p->url.c_str());
				flag = 0;
			}
		}

	} else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		if (!strncmp(key , KEY_VERSION_2.c_str(), 7)) {
			flag = 1;
		}
		json_object_foreach_member(json_node_get_object(node), foreach_json_object, user_data);
	}
}

void MSFDSearchProvider::json_parse(const char *in)
{
	JsonParser *parser = json_parser_new();

	if (json_parser_load_from_data(parser, in, -1, NULL)) {
		JsonNode *node = json_parser_get_root(parser);

		if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
			json_object_foreach_member(json_node_get_object(node), foreach_json_object, this);
		}

	} else {
		dlog_print(DLOG_ERROR, "MSF", "json_parsing error");
	}
}

bool MSFDSearchProvider::stop()
{
	dlog_print(DLOG_INFO, "MSF", "MSFD stop()");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	//if (!searching) {
	//	return false;
	//}
	receive = false;
	close(fd);
	return true;
}

