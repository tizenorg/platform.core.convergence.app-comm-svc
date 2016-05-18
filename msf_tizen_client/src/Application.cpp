#include"Application.h"
#include<curl/curl.h>
#include <dlog.h>
#include "Debug.h"
#include<cstring>
#include"URIparser.h"
#include"Message.h"
#include"Clients.h"

using namespace std;
using namespace uri_parser;

string Application::ROUTE_APPLICATION = "applications/";
string Application::curl_data = "";

class ResultConnectboolCallback : public Result_Base
{
public:
	Client client;
	Result_Base** in_connect_cb  = NULL;

	void onSuccess(bool abc)
	{
		dlog_print(DLOG_INFO, "MSF", "ResultConnectboolCallback success result");
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		if (in_connect_cb != NULL) {
			if (((*in_connect_cb) != NULL) && (!strncmp(client.getId(), "", 1) == 0)) {
				dlog_print(DLOG_INFO, "MSF", "start success result");
				(*in_connect_cb)->onSuccess(client);
			} else {
			}
		}

		delete this;
	}

	void onError(Error)
	{
		dlog_print(DLOG_ERROR, "MSF", "start error result");
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		dlog_print(DLOG_ERROR, "MSF", "start result.");
		//app_pointer->handleSocketClosed();
		delete this;
	}
};

class connectCallback : public Result_Base
{
public:
	ResultConnectboolCallback* start_cb;
	Application* app_pointer = NULL;
	Result_Base** in_connect_cb = NULL;

public:
	void onSuccess(Client abc)
	{
		static Client client12 = abc;
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		if (app_pointer != NULL) {
			start_cb = new ResultConnectboolCallback();
			start_cb->in_connect_cb = in_connect_cb;
			start_cb->client = abc;
			app_pointer->start(start_cb);
		}

		if (in_connect_cb != NULL) {
			if ((*in_connect_cb) != NULL) {
				(*in_connect_cb)->onSuccess(true);
			}
		}
		delete this;
	}

	void onError(Error error)
	{
		// On error
		if (in_connect_cb != NULL) {
			if ((*in_connect_cb) != NULL) {
				(*in_connect_cb)->onError(error);
			}
		}
		delete this;
	}
};

class DisconnectboolCallback : public Result_Base
{
public:
	Application* app_p = NULL;
	Result_Base** in_disconnect_cb = NULL;

	// server application stop success
	void onSuccess(bool abc)
	{
		dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 3");
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		if (abc) {
			dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 4");

			if (in_disconnect_cb != NULL) {
				if ((*in_disconnect_cb) != NULL) {
					app_p->realDisconnect(*in_disconnect_cb);
					//temp->onSuccess(me);
				} else {
				}
			}
		} else {
			dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 5");
			if (in_disconnect_cb != NULL) {
				if ((*in_disconnect_cb) != NULL) {
					Error err(-1, "error", "stop failed");
					DisconnectboolCallback *r = new DisconnectboolCallback();
					r->app_p = this->app_p;
					r->in_disconnect_cb = this->in_disconnect_cb;
					r->onError(err);
				}
			} else {
			}

			//temp->onSuccess(me);
		}

		delete (this);
	}

	void onError(Error error)
	{
		dlog_print(DLOG_ERROR, "MSF", "application disconnect(bool, result) 6");
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		if (in_disconnect_cb != NULL) {
			if ((*in_disconnect_cb) != NULL) {
				dlog_print(DLOG_ERROR, "MSF", "application disconnect(bool, result) 7");
				(*in_disconnect_cb)->onError(error);
			}
		}

		delete (this);
	}
};
Application::Application()
{
	dlog_print(DLOG_INFO, "MSF", "Application()");
	waitForOnReady = false;
	webapp = false;
	install_result_listener = NULL;
	install_result = false;
}

Application::Application(Service *se, string Uri, string Id)
{
	dlog_print(DLOG_INFO, "MSF", "Application(service, uri, id, startargs)");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	map<string, string> startargs = std::map<std::string, std::string>();
	service = se;
	m_uri = Uri;
	ChannelID = Id;
	bool webApp = false;
	uri_parser::URIParse uri2(m_uri);
	if (!(uri2.getURIScheme() == "")) {
		dlog_print(DLOG_INFO, "MSF", "webapp set as true");
		webApp = true;
	}

	this->webapp = webApp;
	this->startArgs = &startargs;
	waitForOnReady = false;
	install_result_listener = NULL;
	install_result = false;
}

Application::Application(Service *se, string Uri, string Id, map<string, string> startargs)
{
	dlog_print(DLOG_INFO, "MSF", "Application(service, uri, id, startargs)");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	service = se;
	m_uri = Uri;
	ChannelID = Id;
	bool webApp = false;
	uri_parser::URIParse uri2(m_uri);
	if (!(uri2.getURIScheme() == "")) {
		dlog_print(DLOG_INFO, "MSF", "webapp set as true");
		webApp = true;
	}

	this->webapp = webApp;
	this->startArgs = &startargs;
	waitForOnReady = false;
	install_result_listener = NULL;
	install_result = false;
}

Application::~Application()
{
	dlog_print(DLOG_INFO, "MSF", "~Application()");
}

bool Application::isConnected()
{
	dlog_print(DLOG_INFO, "MSF", "Channel::isConnected() = %s", Channel::isConnected() ? "true" : "false");
	dlog_print(DLOG_INFO, "MSF", "connected = %s", connected ? "true" : "false");
	//return (Channel::isConnected() && connected);
	return (Channel::isConnected());
}

bool Application::isWebapp()
{
	return webapp;
}

void Application::getinfo(Result_Base *result1)
{
	string uri = service->getUri();
	if (isWebapp()) {
		uri += "webapplication/";
	} else {
		//TODO
		//uri += "applications/" + service->getid() + "/";
	}

	result = result1;
	int ret = curl_application_calling(uri);
	if (ret == -1) {
		if (result1 != NULL)
			result1->onError(Error::create("Failed to get info"));
	}
}

void Application::start(Result_Base *result)
{
	Clients *clientstemp = Channel::getclients();

	if (clientstemp->getHost() == NULL)
		waitForOnReady = true;

	map<string, string> params = getparams();

	invokeMethod(webapp?Message::METHOD_WEB_APPLICATION_START : Message::METHOD_APPLICATION_START, params, result);
}

void Application::stop(Result_Base *result)
{
	map<string, string> params = getparams();
	invokeMethod(webapp?Message::METHOD_WEB_APPLICATION_STOP : Message::METHOD_APPLICATION_STOP, params, result);
}

void Application::install()
{
	int ret;
	if (webapp) {
		string randID = Channel::getUID();
		dlog_print(DLOG_INFO, "MSF", "install call registercallback");
		if (install_result_listener) {
			install_result_listener->onError(Error::create("Unsupported Method"));
		}
		/* Channel::registerCallback(randID,(void*)result,Result_bool); *	
		 *  handleError(randID,Error::create("Unsupported Method"));   */
	} else {
		string Uri = service->getUri().append(ROUTE_APPLICATION).append(m_uri);
		dlog_print(DLOG_INFO, "MSF", "install uri = %s", Uri.c_str());
		ret = Application::curl_install(Uri);
		if (ret == -1) {
			if (install_result_listener)
				install_result_listener->onError(Error::create("Failed to Install"));
		}
	}
}

map<string, string> Application::getparams()
{
	string messagekey = Message::PROPERTY_ID;
	string id = m_uri;
	if (webapp) {
		messagekey = Message::PROPERTY_URL;
	}
	map <string, string> params;
	params[messagekey] = id;
	return params;
}


void Application::connect()
{
	//Channel::connect();
	connect(NULL);
}

void Application::connect(Result_Base *res)
{
	connect(map<string, string>(), res);
}

void Application::connect(map<string, string> attributes, Result_Base *res)
{
	dlog_print(DLOG_INFO, "MSF", "Application::connect(at, res)");

	connectCallback* r = new connectCallback();
	r->in_connect_cb = &connect_cb;
	r->app_pointer = this;

	Channel::connect(attributes, r);
}

void Application::disconnect()
{
	dlog_print(DLOG_INFO, "MSF", "application disconnect()");

	disconnect(true);
}

void Application::disconnect(Result_Base *r)
{
	dlog_print(DLOG_INFO, "MSF", "application disconnect(result)");
	disconnect(true);
}

void Application::disconnect(bool stopOnDisconnect)
{
	dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 1");

	if (stopOnDisconnect) {
	dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 2");
		int numClients = 0;
		numClients = clients->size();
		static Client me ;
		me =  clients->me();

		fprintf(stderr, "\n MYCLIENT ID IS[%s] and Number of clients is [%d]\n", me.getId(), numClients);

		dlog_print(DLOG_INFO, "MSF", "numClients = %d", numClients);
		dlog_print(DLOG_INFO, "MSF", "clients->getHost : %s", clients->getHost() ? "true" : "false");
		dlog_print(DLOG_INFO, "MSF", "strncmp(me.getId(),"",1) : %d", strncmp(me.getId(), "", 1));
		if (((numClients == 2) && (clients->getHost() != NULL) && strncmp(me.getId(), "", 1)) ||
				((numClients == 1) && (strncmp(me.getId(), "", 1))) ||
				 (numClients == 0)) {
			dlog_print(DLOG_INFO, "MSF", "application disconnect(bool, result) 3");
			MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

			//Result_Base *rbool = NULL;;

			DisconnectboolCallback *r = new DisconnectboolCallback();
			r->in_disconnect_cb = &disconnect_cb;
			r->app_p = this;

			//rbool= r1bool;
			MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
			stop(r);
			return;
		}
	}

	realDisconnect(disconnect_cb);
}

void Application:: realDisconnect(Result_Base *result)
{
	Channel::disconnect(result);
}

void Application::invokeMethod(string method, map<string, string> params, string messageID, Result_Base *callback)
{
	dlog_print(DLOG_INFO, "MSF", "invokeMethod ()");
	if (!Channel::isConnected()) {
		dlog_print(DLOG_ERROR, "MSF", "invokeMethod() 1");
		Channel::handleError(messageID, Error::create("Not Connected"));
		return;
	}

	dlog_print(DLOG_INFO, "MSF", "invokeMethod() 2");
	int l = 0;
	string id = webapp?"url":"id";
	char buffer[2000];
	l += sprintf((char *)&buffer, " {\n \"method\": \"%s\",\n \"id\": %s, \n \"params\" : { \n \"%s\": \"%s\" \n } \n }", method.c_str(), messageID.c_str(), id.c_str(), params[id].c_str());
	buffer[l] ='\0';
	dlog_print(DLOG_INFO, "MSF", "invokeMethod() 3");
	dlog_print(DLOG_INFO, "MSF", "invokeMethod() buf = %s", buffer);
	Channel::start_app(buffer, l, messageID);
}

void Application::invokeMethod(string method, map<string, string> params, Result_Base *callback)
{
	string messageID = Channel::getUID();
		dlog_print(DLOG_INFO, "MSF", "invokeMethod call registercallback");
	Channel::registerCallback(messageID, (void*)callback, Result_bool);
	invokeMethod(method, params,  messageID, callback);
}


Application Application::create(Service *service, string uri)
{
	dlog_print(DLOG_INFO, "MSF", "Application::create(service, uri)");
	fprintf(stderr, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	if (!((service == NULL) || (uri == ""))) {
		string id = uri;
		uri_parser::URIParse uri2(uri);
		if (!(uri2.getURIScheme() == "")) {
			id  = uri2.getURIHost();
		}
		//Application *application=new Application(service, uri, id , std::map<std::string, std::string>());
		Application app(service, uri, id , std::map<std::string, std::string>());
		return app;
	} else {
		Application app;
		return app;
	}
}

Application Application::create(Service *service, string uri, string id, map<string, string> startargs)
{
	dlog_print(DLOG_INFO, "MSF", "Application::create(service, uri, id, startargs)");

	if (!((service == NULL) || (uri == "") || (id == ""))) {
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		/*Application *application= new Application(service, uri, id , startargs);	  */
		Application app(service, uri, id , startargs);
		return app;
	} else {
		Application app;
		return app;
	}
}

void Application::createdata_process(string data)
{
	printf("ApplicationInfo curl data : %s", data.c_str());
	ApplicationInfo InfoObj;
	InfoObj.create(data);
	result->onSuccess(InfoObj);
}

size_t Application::createdata(char* buf, size_t size, size_t nmemb, void* up)
{
	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
	if (buf != NULL) {
		curl_data.append(buf, size*nmemb);
	} else {
		dlog_print(DLOG_ERROR, "MSF", "createdata() buf is null");
	}

	return size*nmemb;
}

size_t Application::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	//((char*)ptr)[size*nmemb-1] = 0;
	dlog_print(DLOG_ERROR, "MSF", "read_callback = %s", (char*)ptr);
	if (ptr != NULL)
		static_cast<Application*>(stream)->curl_install_data.append(static_cast<const char*>(ptr), size*nmemb);

	return size*nmemb;
}

int Application::curl_application_calling(string uri)
{
	CURL *curl;
	CURLcode res = CURLE_FAILED_INIT;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl = curl_easy_init();
	dlog_print(DLOG_INFO, "MSF", "uri = %s", uri.c_str());

	if (curl) {
		const char *c = uri.c_str();
		curl_easy_setopt(curl, CURLOPT_URL, c);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Application::createdata);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			dlog_print(DLOG_ERROR, "MSF", "####Application curl ERROR = %d ####", res);
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			createdata_process(curl_data);
		}

		curl_install_data.clear();
		curl_easy_cleanup(curl);

		if (res != CURLE_OK)
			return -1;
		else
			return 0;
	}

	return -1;
}

void Application::foreach_json_object(JsonObject* object, const gchar* key, JsonNode* node, gpointer user_data)
{
	Application* p = static_cast<Application*>(user_data);

	if (!strncmp(key, "ok", 2)) {
		p->install_result = json_node_get_boolean(node);
	} else if (!strncmp(key, "status", 6)) {
		p->errstatus = json_node_get_int(node);
	} else if (!strncmp(key, "message", 7)) {
		p->errMsg = json_node_get_string(node);
	} else if (!strncmp(key, "code", 4)) {
		p->errcode = json_node_get_int(node);
		p->install_result = false;
	}
}

void Application::json_parse(const char* in)
{
	JsonParser* parser = json_parser_new();
	if (json_parser_load_from_data(parser, in, -1, NULL)) {
		JsonNode* node = json_parser_get_root(parser);

		if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
			json_object_foreach_member(json_node_get_object(node), foreach_json_object, this);
		}

	} else {
	}
}

int Application::curl_install(string uri)
{
	CURL *curl;
	CURLcode res = CURLE_FAILED_INIT;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Application::read_callback);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());

		dlog_print(DLOG_ERROR, "MSF", "install curl put send");
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			dlog_print(DLOG_ERROR, "MSF", "####Application curl ERROR = %d ####", res);
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			dlog_print(DLOG_ERROR, "MSF", "####Application curl success ####");
			json_parse(curl_install_data.c_str());

			if (install_result_listener != NULL) {
				if (install_result) {
					install_result_listener->onSuccess(install_result);
				} else {
					string err = "";
					err.append("status:");
					err.append(to_string(errstatus));
					err.append(" message:");
					err.append(errMsg);
					err.append(" code:");
					err.append(to_string(errcode));
					install_result_listener->onError(Error::create(err));
				}
			}
		}



		curl_install_data.clear();

		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}

	if (res != CURLE_OK)
		return -1;
	else
		return 0;
}

void Application::setonInstallListener(Result_Base* listener)
{
	install_result_listener = listener;
}

void Application::unsetonInstallListener()
{
	install_result_listener = NULL;
}
