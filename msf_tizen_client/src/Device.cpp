#include "Device.h"

string Device::DUID_KEY                = "duid";
string Device::MODEL_KEY               = "model";
string Device::MODELNAME_KEY           = "modelName";
string Device::DESCRIPTION_KEY         = "description";
string Device::NETWORK_TYPE_KEY        = "networkType";
string Device::SSID_KEY                = "ssid";
string Device::IP_KEY                  = "ip";
string Device::FIRMWARE_VERSION_KEY    = "firmwareVersion";
string Device::NAME_KEY                = "name";
string Device::ID_KEY                  = "id";
string Device::UDN_KEY                 = "udn";
string Device::RESOLUTION_KEY          = "resolution";
string Device::COUNTRY_CODE_KEY        = "countryCode";
string Device::TYPE						= "type";
string Device::MSF_VERSION				= "msfVersion";
string Device::SMARTHUB_AGREEMENT		= "smartHubAgreement";
string Device::WIFIMAC					= "wifiMac";
string Device::DEVELOPER_MODE			= "developerMode";
string Device::DEVELOPER_IP				= "developerIP";
string Device::OS						= "OS";

void Device::foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data)
{
	Device *p = static_cast<Device*>(user_data);

	if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
		if (!strncmp(key , Device::DUID_KEY.c_str(), 4)) {
			p->duid = json_node_get_string(node);
		} else if (!strncmp(key, Device::MODELNAME_KEY.c_str(), 9)) {
		} else if (!strncmp(key, Device::MODEL_KEY.c_str(), 5)) {
			p->model = json_node_get_string(node);
		} else if (!strncmp(key, Device::DESCRIPTION_KEY.c_str(), 11)) {
			p->description = json_node_get_string(node);
		} else if (!strncmp(key, Device::NETWORK_TYPE_KEY.c_str(), 11)) {
			p->networkType = json_node_get_string(node);
		} else if (!strncmp(key, Device::SSID_KEY.c_str(), 4)) {
			p->ssid = json_node_get_string(node);
		} else if (!strncmp(key, Device::IP_KEY.c_str(), 2)) {
			p->ip = json_node_get_string(node);
		} else if (!strncmp(key, Device::FIRMWARE_VERSION_KEY.c_str(), 15)) {
			p->firmwareVersion = json_node_get_string(node);
		} else if (!strncmp(key, Device::NAME_KEY.c_str(), 4)) {
			p->name = json_node_get_string(node);
		} else if (!strncmp(key, Device::ID_KEY.c_str(), 2)) {
			p->id = json_node_get_string(node);
		} else if (!strncmp(key, Device::UDN_KEY.c_str(), 3)) {
			p->udn = json_node_get_string(node);
		} else if (!strncmp(key, Device::RESOLUTION_KEY.c_str(), 9)) {
			p->resolution = json_node_get_string(node);
		} else if (!strncmp(key, Device::COUNTRY_CODE_KEY.c_str(), 11)) {
			p->countryCode = json_node_get_string(node);
		} else if (!strncmp(key, "running", 7)) {
			//running = json_object_get_boolean(val);
		}
	} else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		json_object_foreach_member(json_node_get_object(node), foreach_json_object, user_data);
	}
}


void Device::json_parse(JsonNode *node)
{
	if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		json_object_foreach_member(json_node_get_object(node), foreach_json_object, this);
	}
}

Device::Device()
{
}

Device::Device(JsonNode *node)
{
	json_parse(node);
}

string Device::getduid()
{
	return duid ;
}

string Device:: getmodel()
{
	return model;
}

string Device::getdescription()
{
	return description;
}

string Device::getnetworkType()
{
	return networkType;
}

string Device::getssid()
{
	return  ssid;
}

string Device::getip()
{
	return ip;
}

string Device::getfirmwareVersion()
{
	return firmwareVersion;
}

string Device::getudn()
{
	return udn;
}

string Device::getresolution()
{
	return resolution;
}

string Device::getcountryCode()
{
	return countryCode;
}

string Device::getname()
{
	return name;
}

string Device::getid()
{
	return id;
}

Device Device::create(JsonNode *node)
{
	Device device =  Device(node);
	return device;
}
