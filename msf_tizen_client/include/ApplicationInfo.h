#ifndef _MSF_APPLICATION_INFO_H_
#define _MSF_APPLICATION_INFO_H_

#include <stdio.h>
#include<iostream>
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>

using namespace std;

class ApplicationInfo
{
	string name;
	string id;
	string msf_version;
	bool running;
	static string PROPERTY_ID;
	static string PROPERTY_STATE;
	static string PROPERTY_NAME;
	static string PROPERTY_VERSION;


public:
	string getID();
	string getName();
	string getVersion();
	bool isRunning();
	ApplicationInfo create(string data);
	ApplicationInfo();
	ApplicationInfo(string iD, bool state, string Name, string Version);
	void json_parse(const char *in);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
};

#endif
