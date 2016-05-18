#include "Service.h"
#include "Search.h"
#include "Result.h"
#include "Channel.h"
#include "Debug.h"
#include "Device.h"
#include "Application.h"
#include<curl/curl.h>

string Service::ID_PROPERTY         = "id";
string Service::VERSION_PROPERTY    = "ve";
string Service::NAME_PROPERTY       = "fn";
string Service::TYPE_PROPERTY       = "md";
string Service::ENDPOINT_PROPERTY   = "se";

Result_Base *Service::Resulturi = NULL;
ServiceInfo Service::serviceval;

Service Service::local_service;

string Service::curl_data = "";

Service::Service()
{
	//Resulturi = NULL;
	//Resultdevice=NULL;
	//devicedata = NULL;
}

/*Service::Service(string Id,string Version,string Name,string Type,string Uri)
{
	id=Id;
	version=Version;
	name=Name;
	type=Type;
	uri=Uri;
}*/

Service::Service(string id1, string version1, string name1, string type1, string endPoint1)
{
	id = id1;
	version = version1;
	name = name1;
	type = type1;
	uri = endPoint1;
}

/*
Search *Service::search()
{
    return Search::getInstance();
}
*/

void Service:: getById(string id, Result_Base *result)
{
	Search search;
	Service service;

	bool status = search.start();
	if (!status)	{
		service = search.getServiceById(id);
	} else {
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		usleep(50000000);
		service = search.getServiceById(id);
		search.stop();
	}

	if (service.getId().length() == 0) {
		if (result != NULL)
			result->onError(Error::create("Service Not Found"));
	} else {
		if (result != NULL)
			result->onSuccess(service);
	}
}

string Service::getName()
{
	return name;
}

string Service::getVersion()
{
	return version;
}

string Service::getType()
{
	return type;
}

string Service::getId()
{
	return id;
}

string Service::getUri()
{
	return uri;
}

void Service::create(char *service_txt)
{
	position posvalues;
	string txt(service_txt);

	posvalues = findServiceValue(ID_PROPERTY, service_txt);
	id = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(VERSION_PROPERTY, service_txt);
	version = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(NAME_PROPERTY, service_txt);
	name = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(TYPE_PROPERTY, service_txt);
	type = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(ENDPOINT_PROPERTY, service_txt);
	uri = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH MDNS WITH URI [%s] [%s] [%d] in %s \n", uri.c_str(), __FUNCTION__, __LINE__, __FILE__);
}

Service Service::create(ServiceInfo servicevalue)
{
	//if (
	string id = servicevalue.infoId;
//	this->id=id;
	string version = servicevalue.infoVersion;
//	this->version=version;
	string name = servicevalue.infoName;
//	this->name=name;
	string type = servicevalue.infotype;
//	this->type=type;
	string endPoint = servicevalue.infoURI;//TODO
//	this->uri=endPoint;
	Service service = Service(id, version, name, type, endPoint);
	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH MSFD WITH URI [%s] [%s] [%d] in %s \n", endPoint.c_str(), __FUNCTION__, __LINE__, __FILE__);
	return service;
}

Service Service::create(map<string, string> serviceMap)
{
	string id = serviceMap.find(ID_PROPERTY)->second;
	string version = serviceMap.find("version")->second;
	string name = serviceMap.find("name")->second;
	string type = serviceMap.find("type")->second;
	string endPoint = serviceMap.find("uri")->second;//TODO
	Service service = Service(id, version, name, type, endPoint);
	return service;
	//return Service;*/
}

position Service::findServiceValue(string substrng, char *strng)
{
	const char *src = strstr(strng, substrng.c_str());
	int startpoint = (src-strng)+sizeof(substrng);
	std::string temp = "\"";
	const char *dest = strstr(src, temp.c_str());
	int endpoint = (dest-strng);
	position values;
	values.start = startpoint;
	values.end = endpoint;
	return values;
}

void Service::getDeviceInfo(Result_Base *dev_result)
{
	dlog_print(DLOG_INFO, "MSF", "getDeviceInfo");
	curl_service_calling(uri, 0, dev_result);
}

Service Service::getLocal(void)
{
	class GetLocalServiceCallback : public Result_Base
	{
		public:
			void onSuccess(Service service)
			{
				dlog_print(DLOG_INFO, "MSF", "service name : %s\n", service.getName().c_str() ? service.getName().c_str() : "name is NULL");
				local_service = service;
			}

			void onError(Error)
			{
				dlog_print(DLOG_ERROR, "MSF", "getLocal() : Fail to get local service info");
			}
	};

	GetLocalServiceCallback r1Service;
	string uri = "http://127.0.0.1:8001/api/v2/";
	getByURI(uri, 0, &r1Service);

	return local_service;
}

void Service::getByURI(string uri,  Result_Base *result)
{
	getByURI(uri, 0, result);
}

void Service::getByURI(string uri, long timeout,  Result_Base *result)
{
	dlog_print(DLOG_INFO, "MSF", "getByURI() uri = %s", uri.c_str());

	Resulturi = result;
	//Resultdevice = NULL;
	curl_service_calling(uri, timeout, NULL);
	Resulturi = NULL;
}

int Service::curl_service_calling(string uri, long timeout, void *dev_result_ptr)
{
	dlog_print(DLOG_INFO, "MSF", "curl_service_calling() 1");
	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl = curl_easy_init();
	if (curl) {
		const char *c = uri.c_str();
		curl_easy_setopt(curl, CURLOPT_URL, c);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		//if (timeout != 0)
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Service::createdata);
		//curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 5000);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		//curl_easy_setopt(curl, CURLOPT_PORT, 8001);
		//curl_easy_setopt(curl, CURLOPT_LOCALPORT, 8888);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			dlog_print(DLOG_ERROR, "MSF", "####Service curl ERROR = %d ####", res);
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			createdata_process("", dev_result_ptr);
		} else {
			createdata_process(curl_data, dev_result_ptr);
			curl_data.clear();
		}

		curl_easy_cleanup(curl);
		dlog_print(DLOG_INFO, "MSF", "curl_service_calling() 4");
	}

	dlog_print(DLOG_INFO, "MSF", "curl_service_calling() 5");

	return 0;
}

size_t Service::createdata(char *buf, size_t size, size_t nmemb, void *up)
{
	dlog_print(DLOG_INFO, "MSF", "createdata() 1");

	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
	if (buf != NULL) {
		//string data;

		curl_data.append(buf, size*nmemb);

		//createdata_process(data, ptr);
	} else {
		dlog_print(DLOG_ERROR, "MSF", "createdata() buf is null");
		//createdata_process(string(), ptr);
	}

	return size*nmemb;
}

void Service::createdata_process(string data, void *dev_result_ptr)
{
	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
	if (data.length() != 0) {
		json_parse_service(data.c_str(), dev_result_ptr);
	} else {
		if (Resulturi != NULL) {
			Resulturi->onError(Error::create("Timeout"));

			//delete (Resulturi);

			//Resulturi = NULL;
		}
		if (dev_result_ptr != NULL) {
			(static_cast<Result_Base*> (dev_result_ptr))->onError(Error::create("Not Found"));
		}
	}
}

void Service::foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data)
{
	if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
		if (!strncmp(key, "id", 2)) {
			serviceval.infoId = json_node_get_string(node);
			dlog_print(DLOG_ERROR, "MSF", "infoId set as %s", serviceval.infoId.c_str());
			//cout<<"\nID : \t"<<serviceval.infoId <<"\n";
		} else if (!strncmp(key, "version", 4)) {
			serviceval.infoVersion = json_node_get_string(node);
			dlog_print(DLOG_ERROR, "MSF", "infoVersion set as %s", serviceval.infoVersion.c_str());
		} else if (!strncmp(key, "name", 7)) {
			serviceval.infoName = json_node_get_string(node);
			dlog_print(DLOG_ERROR, "MSF", "infoName set as %s", serviceval.infoName.c_str());
		} else if (!strncmp(key, "type", 7)) {
			serviceval.infotype = json_node_get_string(node);
			dlog_print(DLOG_ERROR, "MSF", "infotype set as %s", serviceval.infotype.c_str());

		} else if (!strncmp(key, "uri", 7)) {
			serviceval.infoURI = json_node_get_string(node);
			dlog_print(DLOG_ERROR, "MSF", "infoURI set as %s", serviceval.infoURI.c_str());
		}
	} else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		if (!strncmp(key, "device", 7)) {
			MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
			if (user_data != NULL) {
				Device dev = Device::create(node);
				(static_cast<Result_Base *>(user_data))->onSuccess(dev);
			}
		}
	}
}


int Service::json_parse_service(const char *in, void *ptr)
{
	MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);
	dlog_print(DLOG_ERROR, "MSF", "Service::json_parse_service : %s", in);

	JsonParser *parser = json_parser_new();

	if (json_parser_load_from_data(parser, in, -1, NULL)) {
		JsonNode *node = json_parser_get_root(parser);

		if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
			json_object_foreach_member(json_node_get_object(node), foreach_json_object, ptr);
		}

	} else {
		dlog_print(DLOG_ERROR, "MSF", "json_parsing error");
	}

	/*
	dlog_print(DLOG_INFO, "MSF", "Service json_parse_service()");
	void *result_ptr = ptr;
	enum json_type typed;
	json_object_object_foreach(jobj, key, val) {
		typed = json_object_get_type(val);
		switch (typed) {
			case json_type_null:
				break;

			case json_type_boolean:
				break;

			case json_type_double:
				break;

			case json_type_int:
				break;

			case json_type_object:
				dlog_print(DLOG_INFO, "MSF", "Service json_parse_service() 2");
				if (!strncmp(key , "device", 7)) {
					dlog_print(DLOG_INFO, "MSF", "Service json_parse_service() 3");

					MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);	
					if (result_ptr != NULL) {
						dlog_print(DLOG_INFO, "MSF", "Service json_parse_service() 4");
						MSF_DBG("\n Debug Log: SERVICE FOUND THROUGH  WITH URI [%s] [%d] in %s \n", __FUNCTION__, __LINE__, __FILE__);	

						Device dev = Device::create(val);
						(static_cast<Result_Base *> (result_ptr))->onSuccess(dev);
						result_ptr=NULL;
					}
				}
				break;

			case json_type_array:
				break;

			case json_type_string:
				if (!strncmp(key , "id", 2)) {
					serviceval.infoId = json_object_get_string(val);
					//cout<<"\nID : \t"<<serviceval.infoId <<"\n";
				}
				else if (!strncmp(key , "version", 4)) {
					serviceval.infoVersion = json_object_get_string(val);
				}
				else if (!strncmp(key , "name", 7)) {
					serviceval.infoName = json_object_get_string(val);
				}
				else if (!strncmp(key , "type", 7)) {
					serviceval.infotype = json_object_get_string(val);
				}
				else if (!strncmp(key , "uri", 7)) {
					serviceval.infoURI = json_object_get_string(val);
				}
				break;
		}
	}
	*/

	if ((Resulturi!= NULL)) {
		dlog_print(DLOG_INFO, "MSF", "json_parse_service() call onSuccess()");
		Resulturi->onSuccess(create(serviceval));
		//delete (Resulturi);
		//Resulturi=NULL;
	}

	return 0;
}

Application  Service::createApplication(string uri)
{
	if (!(uri == "")) {//check length and return error
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		//Application *application = Application::create(this,uri);
		Application application = Application::create(this, uri);
		return application;
	} else {
		Application app;
		return app;
	}
}

Application Service::createApplication(string uri, string channelId)
{
	if (!((uri == "") || (channelId == ""))) {
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		//Application *application=Application::create(this, uri, channelId, map_type());
		Application application = Application::create(this, uri, channelId, map_type());
		return application;
	} else {
		Application app;
		return app;
	}
}

Application Service::createApplication(string uri, string channelId, map<string, string> startArgs)
{
	if (!((uri == "") || (channelId == ""))) {
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		//Application *application =Application::create(this, uri, channelId, startArgs);
		Application application = Application::create(this, uri, channelId, startArgs);
		return application;
	} else {
		Application app;
		return app;
	}
}

Channel *Service::createChannel(string uri)
{
	Channel *channel = Channel::create(this, uri);
	return channel;
}
