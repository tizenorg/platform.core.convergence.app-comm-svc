/*
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include "IPCServer.h"
#include "CCDebugRemote.h"
#include <curl/curl.h>		//To interact w/ Node server (internal server)
#include <sys/time.h>		//timestamp for VDCurl request
#include "json/writer.h"
#include <net_connection.h>
#include <wifi.h>
#include <system_info.h>
#include <sstream>
#include <syspopup_caller.h>
#include <bundle_internal.h>
#include "bluetooth-api.h"

#define MAC_ADDR_STR_LEN 18
#define DEFAULT_MAC_ADDRESS "00:00:00:00:00:00"

#undef LOG_TAG
#define LOG_TAG "Remote_Daemon"

IPCServer::IPCServer(void)
{
	m_socket = -1;
//	callback = NULL;
}

IPCServer::~IPCServer(void)
{
	Final();
}

int IPCServer::IsAgreedSmartHubTerms(void)
{
	int ret = 0;
	vconf_get_int(VCONFKEY_REMOTE_SERVER_HUB_TERMS, &ret);
	if (ret == 1)
		return ret; // agreement is true

	return 0;  // doesn't agree
}

int IPCServer::Init()
{
	REMOTE_SYSPRINT_INFO("IPCServer Init");

	if ((m_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		REMOTE_PRINT_INFO("IPCServer socket error");
		return -1;
	}

	unlink(PCS_REMOTE_SOCKET_PATH);

	struct sockaddr_un serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_un));
	serveraddr.sun_family = AF_UNIX;
	strncpy(serveraddr.sun_path, PCS_REMOTE_SOCKET_PATH, strlen(PCS_REMOTE_SOCKET_PATH));

	if (bind(m_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
		//Error
		REMOTE_PRINT_INFO("IPCServer socket bind error %d", errno);
		Final();
		return -1;
	}

	if (listen(m_socket, 5) == -1) {	//Max 5 people
		//Error
		REMOTE_PRINT_INFO("IPCServer socket listen error");
		Final();
		return -1;
	}

	if (!Thread::FlagCreate())
		Thread::Create(256 * 1024);

	//cloud app whitelist VDCurl call
	//boost::thread response_thread(&IPCServer::VDCurlMessageSend, this);
	//response_thread.join();

	return 1;
}

void IPCServer::Final(void)
{
	REMOTE_PRINT_INFO("Fianl");

	if (m_socket == -1)
		return;

	shutdown(m_socket, SHUT_RDWR);
	close(m_socket);

	m_socket = -1;

	if (Thread::FlagCreate())
		Thread::Destroy();
}

void IPCServer::t_Main(void)
{
	REMOTE_PRINT_INFO("Enter t_Main");

	SocketProcess();
}

void IPCServer::SocketProcess(void)
{
	while (true) {
		struct sockaddr_un address;
		socklen_t addrSize = sizeof(struct sockaddr_un);

		int sock = (int)accept(m_socket, (struct sockaddr *)&address, &addrSize);

		if (sock  < 0)
			break;

		int fd;
		fd = dup(sock);
		if (fd < 0) {
			REMOTE_PRINT_INFO("File Descriptor is negative, so there is an error\n", __PRETTY_FUNCTION__, __LINE__);
			close(sock);
			return;
		}

		FILE *file = fdopen(fd, "r+");

		if (file == NULL) {
			REMOTE_PRINT_INFO("Can't open file descriptor\n", __PRETTY_FUNCTION__, __LINE__);
			close(fd);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return;
		}

		PacketParse(file, sock);
		fclose(file);
		file = NULL;
		shutdown(sock, SHUT_RDWR);
		close(sock);
	}
}

void* IPCServer::SocketThread(void *arg)
{
	IPCServer *client = (IPCServer *)arg;
	client->SocketProcess();
	return NULL;
}

bool IPCServer::PacketParse(FILE *file, int sock)
{
	int length = 0;
	int result = fscanf(file, "%d", &length);

	if (result <= 0) {
		REMOTE_PRINT_INFO("Error to process PCS command\n", __PRETTY_FUNCTION__, __LINE__);
		return false;
	}

	REMOTE_PRINT_INFO("Get data length %d\n", __PRETTY_FUNCTION__, __LINE__, length);

	//Read real data
	if ((length > 0) && (length < 4096)) {
		char *content = new char[length + 1];
		memset(content, 0, length + 1);
		char param[128] = "";
		snprintf(param, 127, ",%%%ds", length);
		result = fscanf(file, param, content);
		int contentLength = strlen(content);
		if (result <= 0 && length != contentLength) {
			REMOTE_PRINT_INFO("Error to get data, length = %d, get length = %d\n", __PRETTY_FUNCTION__, __LINE__, length, contentLength);
			delete[] content;
			return false;
		}

		//Check content.
		if (strncmp(content, "showpincode,", 12) == 0) {
			int ret = 0;
			char *pin = &(content[12]);
			PinCodeToDisplay = pin;

			char *device_name = NULL;
			device_name = vconf_get_str(VCONFKEY_REMOTE_SERVER_DEVICE_NAME);

			bundle *b = NULL;
			b = bundle_create();
			bundle_add(b, "_PINCODE_", pin);
			bundle_add(b, "_DEVICE_NAME_", device_name);

			ret = syspopup_launch((char*)"rs-pincode-popup", b);
			if (ret < 0)
				REMOTE_PRINT_INFO("Pincode popup Launch Failed", __PRETTY_FUNCTION__, __LINE__);

			bundle_free(b);
			free(device_name);
			device_name = NULL;
//TODO
/*
			PTEvent Event;
			int pinInt = atoi(pin);
			Event.type = CWebServerAppBase::WEBSERVERAPP_EVENT_SHOW_PAIRING_CHALLENGE; // Event Type : WEBSERVERAPP_EVENT_SHOW_PAIRING_CHALLENGE
			Event.param.l[0] = pinInt; // Pincode
			ALMOND::SendEvent(&Event, "samsung.native.webserver"); */
		} else if (strncmp(content, "hidepincode", 11) == 0) {
			int ret = 0;
			ret = syspopup_destroy_all();
			if (ret < 0)
				REMOTE_PRINT_INFO("Pincode popup Destroy Failed", __PRETTY_FUNCTION__, __LINE__);
//TODO
/*
			PTEvent Event;
			Event.type = 0xff0b; // Event Type : WEBSERVERAPP_EVENT_HIDE_PAIRING_CHALLENGE
			ALMOND::SendEvent(&Event, "samsung.native.webserver");*/
		} else if (strncmp(content, "getinfo", 7) == 0) {
			//Get information
			std::string temp = MakeInformation();
			std::string info;
			char szlength[64] = "";
			snprintf(szlength, 63, "%d,", temp.size());
			info.append(szlength);
			info += temp;
//			send(sock, szlength, strlen(szlength), 0);
			REMOTE_PRINT_INFO("Info = %s\n", __PRETTY_FUNCTION__, __LINE__, info.c_str());
			send(sock, info.c_str(), info.size(), 0);
		}
		delete[] content;
	}
	return true;
}

std::string IPCServer::GetPinCode()
{
	REMOTE_PRINT_INFO("PCS pincode is %s\n", __PRETTY_FUNCTION__, __LINE__, PinCodeToDisplay.c_str());
	return PinCodeToDisplay;
}

size_t write_fn(void *ptr, size_t size, size_t nmemb, void *stream)
{
	char *tmp = new char[size * nmemb];
	memset(tmp, 0, size * nmemb);
	memcpy(tmp, ptr, size * nmemb);

	std::string *PINData = (std::string *)stream;
	if (PINData != NULL)
		*PINData += tmp;

	delete[] tmp;
	return size * nmemb;
}

std::string IPCServer::GetPINFromAppID(std::string appID)
{
	static std::string pin;

	if (appID.compare("-1") == 0) {
		unsigned int seed = time(NULL);
		char szPIN[7] = "";
		for (int i = 0; i < 6; i++) {
			int v = rand_r(&seed) % 10;
			szPIN[i] = '0' + v;
		}
		pin = szPIN;
	} else if (appID.compare("-2") == 0) {
		unsigned int seed = time(NULL);
		char szPIN[5] = "";
		for (int i = 0; i < 4; i++) {
			int v = rand_r(&seed) % 10;
			szPIN[i] = '0' + v;
		}
		pin = szPIN;
	} else if (appID.compare("clear") == 0) {
		pin = "";
	}
	return pin;
}

#define MS_URL_NODE_UPDATE	"http://127.0.0.1:8001/ms/1.0/device/info/update"

void IPCServer::UpdateMITNode()
{
	CURL *cu = curl_easy_init();

	if (cu != NULL) {
		REMOTE_PRINT_INFO("Update MIT Node by Curl\n", __PRETTY_FUNCTION__, __LINE__);
		struct curl_slist *headers = NULL;

		std::string data = "{}";

		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(cu, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(cu, CURLOPT_URL, MS_URL_NODE_UPDATE);
		curl_easy_setopt(cu, CURLOPT_POST, 1);
		curl_easy_setopt(cu, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(cu, CURLOPT_POSTFIELDSIZE, data.length());
		curl_easy_setopt(cu, CURLOPT_TIMEOUT_MS, 1000);
//		curl_easy_perform(cu);
		const CURLcode rc = curl_easy_perform(cu);

		if (rc == CURLE_OK)
			REMOTE_PRINT_INFO("Curl Perform Return OK \n", __PRETTY_FUNCTION__, __LINE__);
		else
			REMOTE_PRINT_INFO("Curl Perform Return Fail [%s] \n", __PRETTY_FUNCTION__, __LINE__, curl_easy_strerror(rc));

		curl_easy_cleanup(cu);
	} else {
		REMOTE_PRINT_INFO("Update MIT CURL is failed\n", __PRETTY_FUNCTION__, __LINE__);
	}
}

std::string IPCServer::MakeInformation()
{
	std::string DUID = "";
	std::string ModelCode = "";
	std::string InternalIP = "none";
	std::string ConnectType = "none";
	std::string WiFiSSID;
	std::string FirmwareVersion = "Unknown";
	std::string SmartHubAgreement = "false";
	std::string DeveloperMode = "0";
	std::string DeveloperIP = "0.0.0.0";

	//WiFi SSID & internal IP
	//Network connection check
	connection_h connection;
	if (connection_create(&connection) == CONNECTION_ERROR_NONE) {
		connection_type_e connection_type;
		if (connection_get_type(connection, &connection_type) == CONNECTION_ERROR_NONE) {
			if (connection_type == CONNECTION_TYPE_WIFI) {
				ConnectType = "wireless";

				wifi_ap_h apinfo;
				if (wifi_get_connected_ap(&apinfo) == WIFI_ERROR_NONE) {
					char *szWiFiSSID = NULL;
					if (wifi_ap_get_bssid(apinfo, &szWiFiSSID) == WIFI_ERROR_NONE)
						WiFiSSID = szWiFiSSID;

					if (szWiFiSSID != NULL) {
						free(szWiFiSSID);
						szWiFiSSID = NULL;
					}
					wifi_ap_destroy(apinfo);
				} else {
					REMOTE_PRINT_INFO("IPCServer couldn't get WiFi SSID\n", __PRETTY_FUNCTION__, __LINE__);
				}
			}
		} else {
			REMOTE_PRINT_INFO("IPCServer couldn't get Network type\n", __PRETTY_FUNCTION__, __LINE__);
		}

		//Get IP address
		char *szIP = NULL;
		int ipResult = connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &szIP);
		if (ipResult == CONNECTION_ERROR_NONE) {
			InternalIP = szIP;
		} else {
			if (connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV6, &szIP) == CONNECTION_ERROR_NONE)
				InternalIP = szIP;
			else
				REMOTE_PRINT_INFO("IPCServer couldn't get address of IPv4 & IPv6\n", __PRETTY_FUNCTION__, __LINE__);
		}
		if (szIP != NULL)
			free(szIP);

		//Destroy connection
		if (connection_destroy(connection) != CONNECTION_ERROR_NONE)
			REMOTE_PRINT_INFO("IPCServer destroy connection error\n", __PRETTY_FUNCTION__, __LINE__);
	} else {
		REMOTE_PRINT_INFO("IPCServer couldn't get Network handle\n", __PRETTY_FUNCTION__, __LINE__);
	}

	std::string countryCode;

	char *szCountryCode = NULL;
	szCountryCode =  vconf_get_str(VCONFKEY_REMOTE_SERVER_COUNTRY_ID);
	if (szCountryCode != 0) {
		countryCode = szCountryCode;
		free(szCountryCode);
		szCountryCode = NULL;
	}

	char *szModelCode = NULL;
	szModelCode = vconf_get_str(VCONFKEY_REMOTE_SERVER_MODEL_ID);
	if (szModelCode != 0) {
		ModelCode = szModelCode;
		free(szModelCode);
		szModelCode = NULL;
	}

	char *szDeveloperIP = NULL;
	szDeveloperIP = vconf_get_str(VCONFKEY_REMOTE_SERVER_DEVELOPER_IP);
	if (szDeveloperIP != 0) {
		DeveloperIP = szDeveloperIP;
		free(szDeveloperIP);
		szDeveloperIP = NULL;
	}
	int szDeveloperMode = 0;
	vconf_get_int(VCONFKEY_REMOTE_SERVER_DEVELOPER_MODE, &szDeveloperMode);
	if (szDeveloperMode)
		DeveloperMode = "1";


	//Get Resoulution
	std::string Resolution;

	//Get Resolution
	int width = 1280;
	int height = 720;
	GetPanelResolution(width, height);
	char szResolution[128] = "";
	snprintf(szResolution, 127, "%dx%d", width, height);
	Resolution = szResolution;

	//Get UPNP Device Information
	std::string DeviceName;
	std::string DeviceID;
	std::string Description;
	std::string ModelName;
	std::string UDN;

	if (IsAgreedSmartHubTerms() == 1)
		SmartHubAgreement = "true";
	else
		SmartHubAgreement = "false";

	char bt_mac[MAC_ADDR_STR_LEN];
	ReadBTMAC(bt_mac);

	char wifi_mac[MAC_ADDR_STR_LEN] = {0, };

	ReadWifiMAC(wifi_mac);

	char p2p_mac[MAC_ADDR_STR_LEN] = {0, };
	Read_p2p_MAC(p2p_mac);

	//Make JSON
	std::string info = "{\"DUID\":\"";
	info += UDN.c_str();

	info += "\",\"Model\":\"";
	#if 1 //for platform
	char *p_model_name = NULL;
	int model_name_ret = system_info_get_platform_string("http://tizen.org/system/model_name", &p_model_name);
	if (model_name_ret != SYSTEM_INFO_ERROR_NONE) {
		REMOTE_PRINT_FATAL("Failed to get system information(%d)", model_name_ret);
	} else {
		ModelCode = p_model_name;
		free(p_model_name);
	}
	#endif
	info += ModelCode.c_str();

	info += "\",\"NetworkType\":\"";
	info += ConnectType;

	if (ConnectType.compare("wireless") == 0) {
		info += "\",\"SSID\":\"";
		info += WiFiSSID;
	}

	info += "\",\"IP\":\"";
	info += InternalIP;

	info += "\",\"WifiMac\":\"";
	info += wifi_mac;

	info += "\",\"FirmwareVersion\":\"";
	info += FirmwareVersion;

	info += "\",\"CountryCode\":\"";
	info += countryCode;

	info += "\",\"DeviceName\":\"";
	#if 1 //for platform
	char *p_device_name = NULL;
	p_device_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
	if (p_device_name == NULL) {
		REMOTE_PRINT_FATAL("Failed to get device name");
	} else {
		DeviceName = p_device_name;
		free(p_device_name);
	}
	#endif
	info += DeviceName;

	info += "\",\"DeviceID\":\"";
	info += p2p_mac;

	info += "\",\"ModelDescription\":\"";
	info += Description;

	info += "\",\"ModelName\":\"";
	info += ModelName;

	info += "\",\"UDN\":\"";
	info += UDN;

	info += "\",\"Resolution\":\"";
	info += Resolution;

	info += "\",\"DeveloperMode\":\"";
	info += DeveloperMode;

	info += "\",\"DeveloperIP\":\"";
	info += DeveloperIP;

	info += "\",\"SmartHubAgreement\":\"";
	info += SmartHubAgreement;

	info += "\"}";

	REMOTE_PRINT_DEBUG("Info = %s\n", __PRETTY_FUNCTION__, __LINE__, info.c_str());
	return info;
}

bool IPCServer::GetPanelResolution(int &width, int &height)
{
	//No need to verify return, return false? use default resolution
	int tmpWidth = 0;
	int tmpHeight = 0;

//	system_info_get_value_int(SYSTEM_INFO_KEY_PANEL_RESOLUTION_WIDTH, &tmpWidth);
//	system_info_get_value_int(SYSTEM_INFO_KEY_PANEL_RESOLUTION_HEIGHT, &tmpHeight);
	REMOTE_PRINT_INFO("Panel Resolution = %dx%d\n", __PRETTY_FUNCTION__, __LINE__, tmpWidth, tmpHeight);

	width = tmpWidth;
	height = tmpHeight;
	return true;
}

int IPCServer::cb(int msgType, void *cookie, void *userdata)
{
	return 1;
}

/*
static size_t writeData(char *ptr, size_t size, size_t nmemb, void *stream)
{
	std::ostringstream *pStream = (std::ostringstream*)stream;
	size_t written = size * nmemb;
	pStream->write(ptr, written);
	return written;
}
*/

void IPCServer::ConvertCapitalMac(char *sMac)
{
	int i = 0;
	if (strlen(sMac) != MAC_ADDR_STR_LEN - 1) {
	}
	else {
		for (i = 0; i < MAC_ADDR_STR_LEN; i++) {
			if (sMac[i] >= 'a' && sMac[i] <= 'z')
				sMac[i] = sMac[i] - 0x20;

			if (sMac[i] == '-')
				sMac[i] = ':';
		}
	}
}

void IPCServer::ReadBTMAC(char *sBtmac)
{
    int i = 0;
    bluetooth_device_address_t bt_address;
	bluetooth_get_local_address(&bt_address);

	memset(sBtmac, 0x0, MAC_ADDR_STR_LEN);
	for (i = 0; i < 6; i++) {
		snprintf(sBtmac + (i * 3), 3, "%2X", bt_address.addr[i]);
		if (i < 5)  // Delimeter
			sBtmac[i * 3 + 2] = ':';
	}
	for (i = 0; i < MAC_ADDR_STR_LEN; i++) {
		if (sBtmac[i] == ' ')
			sBtmac[i] = '0';
	}
	ConvertCapitalMac(sBtmac);
}

void IPCServer::ReadWifiMAC(char *sWifimac)
{
	char *mac_addr = NULL;
	int rv = 0;

	// WIFI MAC
	rv = wifi_get_mac_address(&mac_addr);

	if (rv != WIFI_ERROR_NONE) {
		REMOTE_PRINT_INFO("Fail to get WIFI MAC address [%d]", rv);
    } else {
		ConvertCapitalMac(mac_addr);
		if (strncmp(mac_addr, DEFAULT_MAC_ADDRESS, strlen(DEFAULT_MAC_ADDRESS))) {
			memcpy(sWifimac, mac_addr, MAC_ADDR_STR_LEN);
			REMOTE_PRINT_INFO("WIFI mac is %s", sWifimac);
		} else {
			REMOTE_PRINT_INFO("Invalid WIFI MAC loaded = [%s]", mac_addr);
		}
	}

	g_free(mac_addr);
}

char IPCServer::make_p2p_mac(char c)
{
	char convert_c = c;
	if ((convert_c >= 'A') && (convert_c <= 'F')) {
		convert_c = ((((convert_c - 'A') + 10) | 0x02) - 10) + 'A';
	} else if ((convert_c >= '0') && (convert_c <= '9')) {
		convert_c = ((convert_c - '0') | 0x02);
		if (convert_c < 10)
			convert_c = convert_c + '0';
		else
			convert_c = 'A' + (convert_c - 10);
	} else {
		REMOTE_PRINT_FATAL("error : wrong byte for mac!");
	}
	return convert_c;
}

void IPCServer::Read_p2p_MAC(char *p2pmac)
{
	char *temp_addr = NULL;

	memset(p2pmac, 0x0, MAC_ADDR_STR_LEN);

	temp_addr = vconf_get_str(VCONFKEY_WIFI_BSSID_ADDRESS);
	if (temp_addr == NULL) {
		REMOTE_PRINT_FATAL("vconf_get_str Failed for %s", VCONFKEY_WIFI_BSSID_ADDRESS);
	} else {
		memcpy(p2pmac, temp_addr, MAC_ADDR_STR_LEN - 1);
		p2pmac[1] = make_p2p_mac(p2pmac[1]);
		REMOTE_PRINT_INFO("P2P mac is %s", p2pmac);
		free(temp_addr);
	}
}

