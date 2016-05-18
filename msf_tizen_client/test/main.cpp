//#include <msf-api/Search.h>
//#include <msf-api/Service.h>
//#include <msf-api/Clients.h>
//#include <msf-api/Client.h>
//#include <msf-api/Application.h>
//#include <msf-api/ApplicationInfo.h>
//#include <msf-api/Device.h>
//#include <msf-api/Result.h>
//#include <msf-api/Error.h>
//#include <msf-api/Debug.h>
#include "Search.h"
#include "Service.h"
#include "Clients.h"
#include "Client.h"
#include "Application.h"
#include "ApplicationInfo.h"
#include "Device.h"
#include "Result.h"
#include "Error.h"
#include "Debug.h"
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>

/*
 * change this value
 * if you want to use msf c api, set 1
 */


/* availabel appID
'dev-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv':
'dev-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/index.html':
'dev-multiscreen.samsung.com/casteroids/tv/index.html':
'fling-tv.herokuapp.com':
'google.com':
'multiscreen.samsung.com/app-sample-photos/tv/index.html':
'multiscreen.samsung.com/casteroids/tv/index.html':
'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv':
'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/helloworld/tv/index.html':
'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/photoshare/tv':
'prod-multiscreen-examples.s3-website-us-west-1.amazonaws.com/examples/photoshare/tv/index.html':
'yahoo.com':
*/

Application* application = NULL;

int result_arrive = 0;

using namespace std;
class SearchListenerinherit : public SearchListener {
public:
	std::string subject;
	void onStart()
	{
		printf("\nsubject = %s\n", subject.c_str());
		printf("\nSearch onStart");
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
	void onStop()
	{
		printf("\nsubject = %s\n", subject.c_str());
		printf("\nSearch onStop");
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
	void onFound(Service service)
	{
		printf("\nsubject = %s", subject.c_str());
		printf("\nSearch onFound");
		//fprintf(stderr,"\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		printf("\nservice name : %s\n", service.getName().c_str() ? service.getName().c_str() : "name is NULL");
		printf("\nservice uri : %s\n", service.getUri().c_str() ? service.getUri().c_str() : "uri is NULL");
		//fflush(stdout);
		fflush(stdout);
	}
	void onLost(Service service)
	{
		printf("\nsubject = %s", subject.c_str());
		printf("\nSearch onLost");
		printf("\nservice name : %s\n", service.getName().c_str() ? service.getName().c_str() : "name is NULL");
		fflush(stdout);
		//fprintf(stderr,"\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
};

class OnConnectListenerinherit:public OnConnectListener{
public:
	std::string subject;
	void onConnect(Client client) {
		//string id=;
		//bool ishost =client.isHost();
		//int connecttime = client.getConnectTime();
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};
class OnDisconnectListenerinherit:public OnDisconnectListener{
public:
	std::string subject;
	void onDisconnect(Client client) {
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};
class OnClientConnectListenerinherit:public OnClientConnectListener{
public:
	Application* app;

	std::string subject;
	void onClientConnect(Client c) {
		printf("\nsubject = %s", subject.c_str());
		printf("\npublish hello to client %s\n", c.getId());
		fflush(stdout);
		const char* welcome = "hello welcome";
		unsigned char binary[5] = {'a', 'b', 'c', 'd', '\0'};
		app->publish("test_say", welcome, c, binary, 5);

		//fprintf(stderr,"\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
};
class OnClientDisconnectListenerinherit:public OnClientDisconnectListener{
public:
	std::string subject;
	void onClientDisconnect(Client client) {
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
};
class OnMessageListenerinherit:public OnMessageListener{
public:
	std::string subject;
	void onMessage(Message mesg) {
		printf("\nsubject = %s\n", subject.c_str());
		printf("message: [event: %s] ", mesg.m_event.c_str());
		printf("[from: %s] ", mesg.m_from.c_str());

		printf("[msg: %s] ", mesg.m_data.c_str());

		printf("\n");

		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
 	}
};
class OnErrorListenerinherit:public OnErrorListener{
public:
	std::string subject;
	void onError(Client client) {
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};

class ResultClient : public Result_Base
{
public:
	std::string subject;
	void onSuccess(Client abc)
	{
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultClient::onSuccess Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		result_arrive = 1;
 	}
	void onError(Error)
	{
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultClient::onError Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
  	}
};
class ResultBool : public Result_Base
{
public:
	std::string subject;
	void onSuccess(bool abc)
	{
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultBool::onSuccess Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
	void onError(Error e)
	{
		printf("\nsubject = %s", subject.c_str());
		printf("\nerror = %s", e.get_error_message());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultBool::onError Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};
class ResultService : public Result_Base
{
public:
	std::string subject;
	void onSuccess(Service abc)
	{
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultService::onSuccess Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
	void onError(Error)
	{
		printf("\nsubject = %s", subject.c_str());
		fprintf(stderr, "\n [MSF : API] ResultService::onError Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	}
};
class ResultDevice : public Result_Base
{
public:
	std::string subject;
	void onSuccess(Device d)
	{
		printf("\nsubject = %s\n", subject.c_str());
		fflush(stdout);

		printf("id : %s\n", d.getid().c_str());
		printf("countryCode : %s\n", d.getcountryCode().c_str());
		printf("description : %s\n", d.getdescription().c_str());
		printf("dduid : %s\n", d.getduid().c_str());
		printf("firmwareversion : %s\n", d.getfirmwareVersion().c_str());
		printf("ssid : %s\n", d.getssid().c_str());
		printf("ip : %s\n", d.getip().c_str());
		printf("model : %s\n", d.getmodel().c_str());
		printf("name : %s\n", d.getname().c_str());
		printf("networkType : %s\n", d.getnetworkType().c_str());
		printf("resolution : %s\n", d.getresolution().c_str());
		printf("udn : %s\n", d.getudn().c_str());

		fprintf(stderr, "\n [MSF : API] ResultDevice::onSuccess Debug log Function : [%s] and line [%d] in file [%s] and duid [%s] \n", __FUNCTION__, __LINE__, __FILE__, (d.getduid()).c_str());
		result_arrive = 1;
	}
	void onError(Error)
	{
		printf("\nsubject = %s\n", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultDevice::onError Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};
class ResultApplicationInfo : public Result_Base
{
public:
	std::string subject;
	void onSuccess(ApplicationInfo abc)
	{
		printf("\nsubject = %s\n", subject.c_str());
		fflush(stdout);

		printf("app id : %s\n", abc.getID().c_str());
		printf("app name : %s\n", abc.getName().c_str());
		printf("app version : %s\n", abc.getVersion().c_str());
		printf("app running : %s\n", abc.isRunning() ? "true" : "false");
		fprintf(stderr, "\n [MSF : API] ResultApplicationInfo::onSuccess Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
	void onError(Error)
	{
		printf("\nsubject = %s", subject.c_str());
		fflush(stdout);
		fprintf(stderr, "\n [MSF : API] ResultApplicationInfo::onError Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
		result_arrive = 1;
	}
};

#if C_API
int gl_connection_success = TIZEN_MSF_ERROR_NOT_ARRIVE;
int gl_install_success = TIZEN_MSF_ERROR_NOT_ARRIVE;

void device_info_cb(device_handle handle, int result)
{
	printf("device_info_cb\n");
}

void service_found_cb(service_handle handle)
{
	printf("service founded\n");
	get_device_info(handle, device_info_cb);
}

void service_lost_cb(service_handle handle)
{
	printf("service lost\n");
}

void search_start_cb()
{
	printf("search started\n");
}

void search_stop_cb()
{
	printf("search stopped\n");
}

void app_connect_cb(client_handle handle)
{
}

void app_disconnect_cb(client_handle handle)
{
}

void app_client_connect_cb(client_handle handle)
{
}

void app_client_disconnect_cb(client_handle handle)
{
}

void app_message_cb(char* event, char* message, char* from, char* payld)
{
	printf("message cb\n");

	if (event)
		printf("event = %s\n", event);

	if (message)
		printf("message = %s\n", message);

	if (from)
		printf("from = %s\n", from);
}

void app_error_cb()
{
}

void connection_cb(int result, client_handle c_handle, error_handle e_handle)
{
	printf("connection cb\n");
	gl_connection_success = result;
}

void install_cb(int result, error_handle e_handle)
{
	gl_install_success = result;
}

#endif

//Service service;
Search search1;
ResultDevice get_device_result;
OnConnectListenerinherit connection_listener;
ResultApplicationInfo result_applicationInfo;
OnClientConnectListenerinherit client_connect_listener;
OnClientDisconnectListenerinherit client_disconnect_listener;
OnErrorListenerinherit error_listener;
OnDisconnectListenerinherit disconnect_listener;
ResultClient result_client_disconnect;
ResultClient result_client_connect;
ResultBool result_bool_install;
OnMessageListenerinherit msg_listener1;
OnMessageListenerinherit msg_listener2;
ResultApplicationInfo appinfo;
SearchListenerinherit search_listener1;
SearchListenerinherit search_listener2;

void Menu()
{
	printf("\n===============================\n");
	printf("Start search            ==> s\n");
	printf("Display service list    ==> d\n");
	printf("Exit                    ==> Q\n");
	printf("===============================\n");
}

void display_service_list()
{
	list<Service> services = search1.getServices();

	search1.stop();
	cout << "Length of services list is : ";
	cout << "" << services.size() << "\n";

	printf("service list:\n");

	int n = 1;
	for (std::list<Service>::iterator service = services.begin(); service != services.end(); service++) {
		printf("idx : %d, id : %s, name : %s, uri : %s\n", n++, (*service).getId().c_str(), (*service).getName().c_str(), (*service).getUri().c_str());
	}


	char c;
	cout << "\nEnter number to connect : \n";
	cin >> c;

	n = c - 48;
	int n2 = 1;

	for (std::list<Service>::iterator service = services.begin(); service != services.end(); service++) {
		if (n2 != n) {
			n2++;
			continue;
		}

		string appID = "http://www.google.com";
		//string appID = "org.tizen.netflix-app";
		//string appID = "http://multiscreen.samsung.com/app-sample-photos/tv/index.html";
		//string appID="111477001268";
		//string appID="11147700";
		//string appID = "youtube";
		//string channelId = "com.samsung.multiscreen.youtube";
		string channelId = "com.samsung.multiscreen.weblauncher";

		(*service).getDeviceInfo(&get_device_result);
		printf("wait deviceinfo arrive..");
		fflush(stdout);

		while (!result_arrive) {
			sleep(1);
			printf(".");
			fflush(stdout);
		}
		result_arrive = 0;

		application = new Application(&(*service), appID, channelId);

		application->setonConnectListener(&connection_listener);
		client_connect_listener.app = application;
		application->setonClientConnectListener(&client_connect_listener);
		application->setonClientDisconnectListener(&client_disconnect_listener);
		application->setonErrorListener(&error_listener);
		application->setonDisconnectListener(&disconnect_listener);
		application->addOnMessageListener("test_say", &msg_listener1);
		application->addOnAllMessageListener(&msg_listener2);

		application->set_connect_result(&result_client_connect);
		application->connect();
		printf("wait connect..\n");
		fflush(stdout);

		while (!result_arrive) {
			sleep(1);
			printf(".");
			fflush(stdout);
		}
		result_arrive = 0;
		//application->>setConnectionTimeout(5000000);

		//search.stop();
		//usleep(5000000);
		//usleep(5000000);
		//usleep(5000000);
		//Clients cl;
		//cl.create("5fc25160-1dd3-11b2-976d-7b60039367e0",557430,false);
		string ID = "90d105e0-1dd2-11b2-bffa-7536fc73d8d7";
		string event = "test_say";
		//Client c1=application->getclient(ID);
		//const char* messageData = "Hello !";
		//const char* messageData1="Hello sffffWorld!";
		//int messageData=12345;
		//char* target = "host";

		//Application installApplication=(*service).createApplication(appID, channelId);
		application->setonInstallListener(&result_bool_install);
		application->install();
		printf("wait install..\n");
		fflush(stdout);
		while (!result_arrive) {
			sleep(1);
			printf(".");
			fflush(stdout);
		}
		result_arrive = 0;

		//unsigned char a[6] = {'a','x', 'd', 'q', '3', 't'};


		//printf("publish message\n");
		//application->publish(event,messageData, a, 6);//,c1);
		//printf("wait message..\n");
		//fflush(stdout);
		//while (!result_arrive) {
		//	sleep(1);
		//	printf(".");
		//fflush(stdout);
		//}
		//result_arrive = 0;
		//application.publish(event,static_cast<void*>(messageData1));//,c1);

		//sleep(5);
		//printf("\n try disconnect\n");

		//search.stop();


#if 0
		application->getinfo(&appinfo);
		printf("wait appinfo..\n");
		fflush(stdout);
		while (!result_arrive) {
			sleep(1);
			printf(".");
			fflush(stdout);
		}
		result_arrive = 0;
#endif


#if 0
		{
			bool isC = false;
			isC = application->isConnected();
			printf("connected = %s\n", isC ? "true" : "false");
			fflush(stdout);
		}

		{
			bool isW = false;
			isW = application->isWebapp();
			printf("webapp = %s\n", isW ? "true" : "false");
			fflush(stdout);
		}

#endif
		// Clients Client test
		{
			Clients* cs = application->getclients();

			if (cs == NULL) {
				printf("No clients\n");
			} else {
				std::list<Client> cl = cs->lists();
				int cs_index = 0;
				Client cli;
				printf("clients size = %d\n", cs->size());

				for (auto cs_itr = cl.begin(); cs_itr != cl.end(); cs_itr++) {
					printf("client %d\n", cs_index++);
					printf("client id :  %s\n", (*cs_itr).getId());
					printf("client clients.get().getId() : %s\n", cs->get((*cs_itr).getId()).getId());
					printf("client isHost :  %s\n", (*cs_itr).isHost() ? "true" : "false");
					printf("client connectTIme :  %lld\n", (*cs_itr).getConnectTime());
					Client* host = cs->getHost();
					printf("host = %s\n", host ? host->getId() : "no host");
					Channel* cha = NULL;
					cha = (*cs_itr).getChannel();
					if (cha != NULL)
						printf("client channel uri = %s\n", cha->getChannelUri(NULL).c_str());
					//printf("clients channel uri = %s\n", cs->getChannel()->getChannelUri(NULL).c_str());
					printf("channel uri : %s\n", application->getChannelUri(NULL).c_str());
				}


				application->publish(event, "Hello Im newbe", cl);//,c1);
			}
		}

		{
			cout << "\nEnter a to disconnect : \n";
			cin >> c;

			application->set_disconnect_result(&result_client_disconnect);
			application->disconnect(true);
			printf("wait disconnect..\n");
			fflush(stdout);
			while (!result_arrive) {
				sleep(1);
				printf(".");
				fflush(stdout);
			}
			result_arrive = 0;
		}

		exit(0);
	}
}

int test_thread(GIOChannel *source, GIOCondition condition, gpointer data)
{
	get_device_result.subject = "getDeviceInfo";
	connection_listener.subject = "connect_listen";
	result_applicationInfo.subject = "appinfo";
	client_connect_listener.subject = "client_connect_listen";
	client_disconnect_listener.subject = "client_disconnect_listen";
	error_listener.subject = "error_listen";
	disconnect_listener.subject = "disconnect_listen";
	result_client_connect.subject = "connect_result";
	result_client_disconnect.subject = "disconnect_result";
	result_bool_install.subject = "install_result";
	msg_listener1.subject = "msg_listener 1";
	msg_listener2.subject = "msg_listener 2";
	appinfo.subject = "app info";
	search_listener1.subject = "search_listener1";
	search_listener2.subject = "search_listener2";


	search1.setSearchListener(&search_listener1);

	int rv;
	char a[10];
	GMainLoop *mainloop = reinterpret_cast<GMainLoop*>(data);

	printf("Event received from stdin\n");

	rv = read(0, a, 10);
	if (rv <= 0) {
		printf("read result error : %d\n", rv);
		rv = 1;
	}

	if (a[0] == '\n' || a[0] == '\r') {
		printf("Press ENTER to show options menu.......\n");
		Menu();
	}

	switch (a[0]) {
	case 's':
		search1.start();
		break;
	case 'd':
		display_service_list();
		break;
	case 'Q':
		g_main_loop_quit(mainloop);
		break;
	default :
		return true;
	}

	if (rv >= 1)
		printf("\nOperation Succeeded!\n");
	else
		printf("\nOperation Failed!\n");

	return TRUE;
}


int main(int argc, char** argv) {
	GMainLoop *mainloop;
	mainloop = g_main_loop_new(NULL, FALSE);

	GIOChannel *channel = g_io_channel_unix_new(0);
	g_io_add_watch(channel, static_cast<GIOCondition>(G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL),
			test_thread, mainloop);

	printf("Test Thread created...\n");

	g_main_loop_run(mainloop);

	printf("Test Application Terminated\n");
	g_main_loop_unref(mainloop);

	return 0;
}
