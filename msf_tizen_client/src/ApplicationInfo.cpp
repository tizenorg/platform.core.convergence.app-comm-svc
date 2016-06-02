#include"ApplicationInfo.h"

string ApplicationInfo::PROPERTY_ID = "id";
string ApplicationInfo::PROPERTY_STATE = "running";
string ApplicationInfo::PROPERTY_NAME = "name";
string ApplicationInfo::PROPERTY_VERSION = "version";

ApplicationInfo::ApplicationInfo()
{
}

ApplicationInfo::ApplicationInfo(string iD, bool state, string Name, string Version)
{
	id = iD;
	running = state;
	name = Name;
	msf_version = Version;
}

string ApplicationInfo::getID()
{
	return  id;
}

string ApplicationInfo::getName()
{
	return name;
}

string ApplicationInfo::getVersion()
{
	return msf_version;
}

bool ApplicationInfo::isRunning()
{
	return running;
}

void ApplicationInfo::foreach_json_object(JsonObject* object, const gchar* key, JsonNode* node, gpointer user_data)
{
	ApplicationInfo* p = static_cast<ApplicationInfo*>(user_data);

	if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
		if (!strncmp(key, ApplicationInfo::PROPERTY_STATE.c_str(), 7)) {
			p->running = json_node_get_boolean(node);
		} else if (!strncmp(key, ApplicationInfo::PROPERTY_ID.c_str(), 2)) {
			p->id = json_node_get_string(node);
		} else if (!strncmp(key, ApplicationInfo::PROPERTY_NAME.c_str(), 4)) {
			p->name = json_node_get_string(node);
		} else if (!strncmp(key, ApplicationInfo::PROPERTY_VERSION.c_str(), 7)) {
			p->msf_version = json_node_get_string(node);
		}
	} else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		json_object_foreach_member(json_node_get_object(node), foreach_json_object, user_data);
	}
}

void ApplicationInfo::json_parse(const char* in)
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

ApplicationInfo ApplicationInfo::create(string data)
{
	json_parse(data.c_str());

	ApplicationInfo appinfo(id, running, name, msf_version);

	return appinfo;
}
