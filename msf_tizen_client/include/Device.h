#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>

using namespace std;

class Device
{
	static string DUID_KEY;
	static string MODEL_KEY;
	static string MODELNAME_KEY;
	static string DESCRIPTION_KEY;
	static string NETWORK_TYPE_KEY;
	static string SSID_KEY;
	static string IP_KEY;
	static string FIRMWARE_VERSION_KEY ;
	static string NAME_KEY ;
	static string ID_KEY ;
	static string UDN_KEY ;
	static string RESOLUTION_KEY;
	static string COUNTRY_CODE_KEY;
	static string TYPE;
	static string MSF_VERSION;
	static string SMARTHUB_AGREEMENT;
	static string WIFIMAC;
	static string DEVELOPER_MODE;
	static string DEVELOPER_IP;
	static string OS;

	string duid;
	string model;
	string description;
	string networkType;
	string ssid;
	string ip;
	string firmwareVersion;
	string name;
	string id;
	string udn;
	string resolution;
	string countryCode;
	string type;
	string msf_version;
	string smarthub_agreement;
	string wifimac;
	string developer_mode;
	string deveoper_ip;
	string os;

public:
	string getduid();
	string getmodel();
	string getdescription();
	string getnetworkType();
	string getssid();
	string getip();
	string getfirmwareVersion();
	string getname();
	string getudn();
	string getresolution();
	string getcountryCode();
	string getid();

	Device();
	Device(JsonNode *node);
	static Device create(JsonNode *node);
	void json_parse(JsonNode *node);
	static void foreach_json_object(JsonObject *object, const gchar *name, JsonNode *node, gpointer user_data);
};

#endif
