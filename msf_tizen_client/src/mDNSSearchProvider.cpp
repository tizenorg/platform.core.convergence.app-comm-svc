#include"mDNSSearchProvider.h"
#include <stdlib.h>
#include "Debug.h"
#include <list>
#include "Service.h"

#define SERVICE_TYPE "_samsungmsf._tcp"

dnssd_service_h g_service = 0;

mDNSSearchProvider::~mDNSSearchProvider()
{
	MSF_DBG("mDNS off.");
}

static const char *dnssd_error_to_string(dnssd_error_e error)
{
	switch (error) {
		case DNSSD_ERROR_NONE:
			return "DNSSD_ERROR_NONE";
		case DNSSD_ERROR_OUT_OF_MEMORY:
			return "DNSSD_ERROR_OUT_OF_MEMORY";
		case DNSSD_ERROR_INVALID_PARAMETER:
			return "DNSSD_ERROR_INVALID_PARAMETER";
		case DNSSD_ERROR_NOT_SUPPORTED:
			return "DNSSD_ERROR_NOT_SUPPORTED";
		case DNSSD_ERROR_NOT_INITIALIZED:
			return "DNSSD_ERROR_NOT_INITIALIZED";
		case DNSSD_ERROR_ALREADY_REGISTERED:
			return "DNSSD_ERROR_ALREADY_REGISTERED";
		case DNSSD_ERROR_NAME_CONFLICT:
			return "DNSSD_ERROR_NAME_CONFLICT";
		case DNSSD_ERROR_SERVICE_NOT_RUNNING:
			return "DNSSD_ERROR_SERVICE_NOT_RUNNING";
		case DNSSD_ERROR_OPERATION_FAILED:
			return "DNSSD_ERROR_OPERATION_FAILED";
		default:
			return "UNSUPPORTED_ERROR";
	}
}

mDNSSearchProvider::mDNSSearchProvider()
{
	MSF_DBG(" [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] ", __FUNCTION__, __LINE__, __FILE__);
	SearchProvider();
}

mDNSSearchProvider::mDNSSearchProvider(Search *sListener) : SearchProvider(sListener)
{
	MSF_DBG(" [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] ", __FUNCTION__, __LINE__, __FILE__);
}

SearchProvider mDNSSearchProvider::create()
{
	return (SearchProvider)mDNSSearchProvider();
}

SearchProvider mDNSSearchProvider::create(Search *searchListener)
{
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	return (SearchProvider)mDNSSearchProvider(searchListener);
}

void mDNSSearchProvider::addService(Service service)
{
	this->SearchProvider::addService(service);
}

void extract_service_info(ServiceInfo &info, string text)
{
	int id_pos = text.find("id=");
	int remote_pos = text.find("remote=");
	int se_pos = text.find("se=");
	int ve_pos = text.find("ve=");
	int fn_pos = text.find("fn=");
	int md_pos = text.find("md=");

	char c_text[100];
	int j = 0;

	MSF_DBG("TxT Record : %s", text.c_str());

	for(int i = id_pos + 3; remote_pos > i; i++, j++) {
		c_text[j] = text[i];
	}

	c_text[j-1] = '\0';
	info.infoId = c_text;
	MSF_DBG("ID : %s", c_text);

	j = 0;
	for(int i = ve_pos + 3; md_pos > i; i++, j++) {
		c_text[j] = text[i];
	}

	c_text[j-1] = '\0';
	info.infoVersion = c_text;
	MSF_DBG("Version : %s", c_text);

	j = 0;
	for(int i = fn_pos + 3; se_pos > i; i++, j++) {
		c_text[j] = text[i];
	}

	c_text[j-1] = '\0';
	info.infoName = c_text;
	MSF_DBG("Name : %s", c_text);

	j = 0;
	for(unsigned int i = se_pos + 3; text.size() > i; i++, j++) {
		c_text[j] = text[i];
	}

	c_text[j] = '\0';
	info.infoURI = c_text;
	MSF_DBG("Uri : %s", c_text);
}

static void dnssd_browse_reply(dnssd_service_state_e service_state, dnssd_service_h remote_service, void *user_data)
{
	MSF_DBG("[MSF : API] Debug log Function : [%s] and line [%d] in file [%s] ", __FUNCTION__, __LINE__, __FILE__);
	mDNSSearchProvider *provider = (mDNSSearchProvider*)user_data;
	int rv = 0;
	MSF_DBG("Handler       : %u", remote_service);
	MSF_DBG("State         : ");
	switch (service_state) {
		case DNSSD_SERVICE_STATE_AVAILABLE:
			MSF_DBG("Available");
			break;
		case DNSSD_SERVICE_STATE_UNAVAILABLE:
			MSF_DBG("Un-Available");
			break;
		case DNSSD_SERVICE_STATE_NAME_LOOKUP_FAILED:
			MSF_DBG("Lookup failure for service name");
			break;
		case DNSSD_SERVICE_STATE_HOST_NAME_LOOKUP_FAILED:
			MSF_DBG("Lookup failure for host name and port number");
			break;
		case DNSSD_SERVICE_STATE_ADDRESS_LOOKUP_FAILED:
			MSF_DBG("Lookup failure for IP address");
			break;
		default:
			MSF_DBG("Unknown Browse State");
			break;
	}

	char *name = NULL;
	char *type = NULL;

	rv = dnssd_service_get_name(remote_service, &name);
	if (rv == DNSSD_ERROR_NONE && name != NULL)
		MSF_DBG("Service Name  : %s", name);

	rv = dnssd_service_get_type(remote_service, &type);
	if (rv == DNSSD_ERROR_NONE && type != NULL)
		MSF_DBG("Service Type  : %s", type);

	if (service_state == DNSSD_SERVICE_STATE_AVAILABLE) {
		char *ip_v4_address = NULL;
		char *ip_v6_address = NULL;
		char *txt_record = NULL;
		unsigned short txt_len = 0;
		int port = 0;

		rv = dnssd_service_get_ip(remote_service, &ip_v4_address, &ip_v6_address);
		if (rv  == DNSSD_ERROR_NONE) {
			if (ip_v4_address)
				MSF_DBG("IPv4 Address  : %s", ip_v4_address);
			if (ip_v6_address)
				MSF_DBG("IPv6 Address  : %s", ip_v6_address);
		}

		rv = dnssd_service_get_port(remote_service, &port);
		MSF_DBG("Port          : %d\n", port);

		dnssd_service_get_all_txt_record(remote_service, &txt_len,
				(void **)&txt_record);
		MSF_DBG("TXT Record: %s", txt_record);

		string temp_str(txt_record, txt_len);

		ServiceInfo service_info; //id, version, name, type, Uri
		MSF_DBG("txt_len %d", txt_len);
		if (txt_len > 100) {
			extract_service_info(service_info, temp_str);
			service_info.infotype = SERVICE_TYPE;
			Service service = Service::create(service_info);
			provider->addService(service);
			MSF_DBG("service : id( %s ) registerd.", service_info.infoId.c_str());
		}

		free(ip_v4_address);
		free(ip_v6_address);
		free(name);
		free(type);
		free(txt_record);
	}
}

void mDNSSearchProvider::start()
{
	if (g_service != 0) {
		dnssd_stop_browsing_service(g_service);
		g_service = 0;
	}

	int rv = dnssd_initialize();
	if (rv != DNSSD_ERROR_NONE) {
		MSF_DBG("mDNS is not initialzed.");
		return;
	} else {
		MSF_DBG("mDNS is initialzed successfully.");
	}

	rv = dnssd_start_browsing_service(SERVICE_TYPE, &g_service, dnssd_browse_reply, this);
	if (rv != DNSSD_ERROR_NONE) {
		MSF_DBG("Failed to browse for dns service, error %s", dnssd_error_to_string((dnssd_error_e)rv));
	} else {
		MSF_DBG("Succeeded to browse for dns service.");
	}
}

bool mDNSSearchProvider::stop()
{
	int rv;

	if (g_service != 0) {
		rv = dnssd_stop_browsing_service(g_service);
		g_service = 0;
		if (rv != DNSSD_ERROR_NONE) {
			MSF_DBG("Failed to stop browse dns service %s", dnssd_error_to_string((dnssd_error_e)rv));
			return false;
		} else {
			MSF_DBG("Successfully stopped browsing dns service");
			dnssd_deinitialize();
			return true;
		}
	} else {
		MSF_DBG("Browsing is aleady stopped.");
		return false;
	}
}
