#ifndef _MSF_APPLICATION_H_
#define _MSF_APPLICATION_H_

#include"Result.h"
#include"ApplicationInfo.h"
#include"Channel.h"
#include"Service.h"


class Application:public Channel
{
public:
	bool webapp;
	static string ROUTE_APPLICATION;
	static string ROUTE_WEBAPPLICATION;
	static string curl_data;
	bool install_result = false;
	Result_Base *install_result_listener = NULL;

	string createdata_data;
	string curl_install_data;
	typedef std::map<std::string, std::string> map_null;
	map<string, string> *startArgs;
	int curl_install(string uri);
	int curl_application_calling(string uri);
	map<string, string> getparams();
	void invokeMethod(string method, map<string, string> params, Result_Base *);
	void invokeMethod(string method, map<string, string> params,  string messageID, Result_Base *);
	void realDisconnect(Result_Base *result);

public:
	Application();
	Application(Service *service, string uri, string id);
	Application(Service *service, string uri, string id, map<string, string> startArgs);
	virtual ~Application();
	Result_Base *result ;
	bool isConnected();
	bool isWebapp();
	void createdata_process(string data);
	void getinfo(Result_Base *result);
	void connect();
	void connect(Result_Base *r);
	void connect(map<string, string> attributes, Result_Base *r);
	void disconnect();
	void disconnect(Result_Base *r);
	void disconnect(bool stopOnDisconnect);
	void install();
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t createdata(char *buf, size_t size, size_t nmemb, void *up);
	static Application create(Service *service, string uri);
	static Application create(Service *service, string uri, string id, map<string, string>);
	void start(Result_Base *result);
	void stop(Result_Base *result);
	void json_parse(const char *in);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
	void setonInstallListener(Result_Base *listener);
	void unsetonInstallListener();
};

#endif
