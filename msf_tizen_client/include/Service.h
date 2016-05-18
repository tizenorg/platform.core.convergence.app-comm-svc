#ifndef _MSF_SERVICE_H
#define _MSF_SERVICE_H

#include <string>
#include <map>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>


using namespace std;

class Application;
class Search;
class Result_Base;
class Channel;

struct position {
	int start;
	int end;
};

struct ServiceInfo {
	string infoId;
	string infoVersion;
	string infoName;
	string infotype;
	string infoURI;
};

class Service
{
	static  string ID_PROPERTY;
	static  string VERSION_PROPERTY;
	static  string NAME_PROPERTY;
	static  string TYPE_PROPERTY;
	static  string ENDPOINT_PROPERTY;
	string id = "";
	string version = "";
	string name = "";
	string type = "";
	string uri = "";
	static string curl_data;
	typedef std::map<std::string, std::string> map_type;
	static Service local_service;
	void *result_ptr;

public:
	Service();
	Service(string, string, string, string, string);
	static ServiceInfo serviceval;
	static Result_Base *Resulturi;
	Result_Base *Resultdevice;

	position findServiceValue(string, char *);
	void getDeviceInfo(Result_Base*);
	void create(char*);
	string getUri();
	string getName();
	string getType();
	string getId();
	string getVersion();
	static Service getLocal(void);
	static void getByURI(string, Result_Base*);
	static void getByURI(string, long, Result_Base *result);
	static void getById(string id, Result_Base *result);
	static int curl_service_calling(string uri, long, void *);
	//static Search *search();
	static size_t createdata(char *buf, size_t size, size_t nmemb, void *up);
	static void createdata_process(string data,  void *);
	static int json_parse_service(const char *in,  void *);
	static Service create(ServiceInfo);
	static Service create(map<string, string>);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
	Channel *createChannel(string uri);
	Application createApplication(string uri);
	Application createApplication(string uri, string channelId);
	Application createApplication(string uri, string channelId, map<string, string> startArgs) ;
};
#endif
