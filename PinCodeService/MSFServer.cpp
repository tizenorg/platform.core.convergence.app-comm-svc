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
#include "MSFServer.h"
#include "CCDebugRemote.h"
#include <app_manager.h>
#include "app_control.h"
#include "system_info.h" // to use capi-system-info package api
#include "MSFVconfDefine.h"
#include "MSFSend.h"
#include "REMOTEString.h"
#include "RemoteLogger.h"
#include "REMOTEApp.h"
#include <aul.h>
#include <unistd.h>

int callee_pid;		// callee's pid. for launching and terminating the corresponding app

MSFServer::MSFServer(void)
{
	m_socket = -1;
	recvThread = 0;
}

MSFServer::~MSFServer(void)
{
	Final();
}

int MSFServer::Init()
{
	REMOTE_SYSPRINT_INFO("[%s][%d] Init", __PRETTY_FUNCTION__, __LINE__);

	if ((m_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		REMOTE_PRINT_INFO("[%s][%d] IPCServer socket error", __PRETTY_FUNCTION__, __LINE__);
		return -1;
	}

	if ( 0 == unlink(MSF_SOCKET_PATH))
		REMOTE_PRINT_INFO("[%s][%d] MSFServer remove file", __PRETTY_FUNCTION__, __LINE__);
	else
		REMOTE_PRINT_WARN("[%s][%d] MSFServer failed to remove file!", __PRETTY_FUNCTION__, __LINE__);

	struct sockaddr_un serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_un));
	serveraddr.sun_family = AF_UNIX;
	strncpy(serveraddr.sun_path, MSF_SOCKET_PATH, strlen(MSF_SOCKET_PATH));

	if (bind(m_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
		//Error
		REMOTE_PRINT_WARN("[%s][%d] MSFServer socket bind error ", __PRETTY_FUNCTION__, __LINE__);
		goto error;
	}

	if (listen(m_socket, 1) == -1) {	//Max 5 people
		//Error
		REMOTE_PRINT_WARN("[%s][%d] MSFServer socket listen error", __PRETTY_FUNCTION__, __LINE__);
		goto error;
	}

	if (launchLock.FlagCreate() == false)
		launchLock.Create();

	StartRecv();
	return 1;

error:
	Final();
	return -1;
}

void MSFServer::Final(void)
{
	if (m_socket == -1)
		return;

	REMOTE_PRINT_INFO("[%s][%d] ClientSocket.Close %d", __PRETTY_FUNCTION__, __LINE__, m_socket);
	shutdown(m_socket, SHUT_RDWR);
	close(m_socket);

	if (launchLock.FlagCreate() == true)
		launchLock.Destroy();

	m_socket = -1;
	REMOTE_PRINT_INFO("[%s][%d] ClientSocket.Close() Exit ~~", __PRETTY_FUNCTION__, __LINE__);
}

int MSFServer::StartRecv(void)
{
	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&tattr, 128 * 1024);
	int result = pthread_create(&recvThread, &tattr, MSFServer::SocketThread, this);
	pthread_attr_destroy(&tattr);
	return result;
}

void MSFServer::SocketProcess(void)
{
	while (true) {
		struct sockaddr_un address;
		socklen_t addrSize = sizeof(struct sockaddr_un);
		int sock = (int)accept(m_socket, (struct sockaddr *)&address, &addrSize);

		if (sock  < 0)
			break;

		int fd;
		fd = dup(sock);
		if(fd < 0) {
			REMOTE_PRINT_WARN("[%s][%d] File Descriptor is negative, so there is an error", __PRETTY_FUNCTION__, __LINE__);
			close(sock);
			continue;
		}

		FILE *file = fdopen(fd, "r+");

		if (file == NULL) {
			REMOTE_PRINT_WARN("[%s][%d] Can't open file descriptor", __PRETTY_FUNCTION__, __LINE__);
			close(fd);
			close(sock);
			continue;
		}

		PacketParse(file, sock);
		close(fd);
		shutdown(sock, SHUT_RDWR);
		close(sock);
		fclose(file);
	}
}

void* MSFServer::SocketThread(void *arg)
{
	MSFServer *client = (MSFServer *)arg;
	client->SocketProcess();
	return NULL;
}

bool MSFServer::PacketParse(FILE *file, int sock)
{
	int status = 0 ;
	vconf_get_int(VCONFKEY_REMOTE_SERVER_MLS_STATE_DB, &status);
	REMOTE_PRINT_INFO("**************************** Checking for the MLS state*************************************** %d ", status);

	if (status)
		return false;

	long length = 0;
	int result = fscanf(file, "%ld,", &length);
	if (result <= 0) {
		REMOTE_PRINT_INFO("[%s][%d] Error to process MSFServer command", __PRETTY_FUNCTION__, __LINE__);
		return false;
	}

	REMOTE_PRINT_INFO("[%s][%d] Get data length %ld", __PRETTY_FUNCTION__, __LINE__, length);

	//Read real data
	if ((length > 0) && (length < 8192)) {
		char *content = new char[length + 1];
		memset(content, 0, length + 1);
		int contentLength = fread(content, 1, length, file);

		if (length != contentLength) {
			REMOTE_PRINT_INFO("[%s][%d] Error to get data, length = %ld, get length = %d", __PRETTY_FUNCTION__, __LINE__, length, contentLength);
			delete[] content;
			return false;
		}

		ParseJSON(sock, content);
		delete[] content;
	} else {
		return false;
	}
	return true;
}

void MSFServer::ParseJSON(int sock, std::string contents)
{
	std::string json_message = std::string(contents);

	REMOTE_PRINT_INFO("[%s][%d] Get data %s", __PRETTY_FUNCTION__, __LINE__, json_message.c_str());
	try {
		// Parse json_message
		Json::Value root;
		Json::Reader reader;
		Json::FastWriter writer;
		bool parsingError = reader.parse(json_message, root, false);
		if (!parsingError) {
			REMOTE_PRINT_INFO("[%s][%d] Error to parse json data, json message = %s", __PRETTY_FUNCTION__, __LINE__, json_message.c_str());
			CMSFSend::SendErrorMessage(sock, MSF_JSON_ERROR, std::string("Error to parse JSON data."), json_message, Json::Value(std::string("")));
			return;
		}

		if (root["id"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, "", "id");
			return;
		}
		std::string id = root["id"].asString();

		if (root["method"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "method");
			return;
		}
		std::string method = root["method"].asString();
		Json::Value params = root["params"];	// 'params' can be NULL.

		if (params.isNull()) {
			REMOTE_PRINT_INFO("[%s][%d] Get data %s", __PRETTY_FUNCTION__, __LINE__, json_message.c_str());
			CMSFSend::SendErrorMessage(sock, MSF_JSON_ERROR, std::string("Error to parse JSON data."), json_message, Json::Value(std::string("")));
		} else {
			if (launchLock.FlagCreate())
				launchLock.Lock();

			if (method.compare("application.getApplication") == 0) {
				DoGetApplication(sock, params, id);
			} else if (method.compare("application.partyMode") == 0) {
				DoCheckPartyMode(sock, params, id);
			} else if (method.compare("application.launchApplication") == 0) {
				DoLaunchApplication(sock, params, id);
			} else if (method.compare("application.stopApplication") == 0) {
				DoStopApplication(sock, params, id);
			} else if (method.compare("application.installApplication") == 0) {
				DoInstallApplication(sock, params, id);
			} else if (method.compare("application.aclPairing") == 0) {
				DoACLPairing(sock, params, id);
			} else if (method.compare("application.getCastingAppsInfo") == 0) {
			} else {
				REMOTE_PRINT_INFO("[%s][%d] Get data %s", __PRETTY_FUNCTION__, __LINE__, json_message.c_str());
				CMSFSend::SendErrorMessage(sock, MSF_JSON_ERROR, std::string("Error to parse JSON data."), json_message, Json::Value(std::string("")));
			}
			if (launchLock.FlagCreate())
				launchLock.Unlock();
		}
	} catch (Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : params/ method / id ");
	}
}

void MSFServer::DoGetApplication(int sock, Json::Value& params, std::string id)
{
	std::string widgetname;
	std::string ver;
	try {
		REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);
		if (params["appId"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "id");
			return;
		}
		std::string appID = params["appId"].asString();
		REMOTE_PRINT_INFO("[%s][%d] getApplication ID = %s", __PRETTY_FUNCTION__, __LINE__,  appID.c_str());
	//	was_app_info_s widget_info;
		Json::Value appInfo = Json::Value(Json::objectValue);
		Json::Value resp;

		//Make JSON response message
		appInfo["appId"] = Json::Value(appID);

		if (widgetList.find(appID) != widgetList.end()) {
			ver = widgetList[appID].version;
			widgetname = widgetList[appID].name;
		}

		REMOTE_PRINT_INFO("[%s][%d] version = %s, widget name = %s", __PRETTY_FUNCTION__, __LINE__, ver.c_str(), widgetname.c_str());

		if (ver.size() > 0)
			appInfo["version"] = Json::Value(ver);
		else
			appInfo["version"] = Json::Value("none");

		if (widgetname.size() > 0)
			appInfo["name"] = Json::Value(widgetname);
		else
			appInfo["name"] = Json::Value("none");

		bool running = false;

		if (aul_app_is_running(appID.c_str()))
			running = true;

		appInfo["running"] = Json::Value(running);
		CMSFSend::SendRespMessage(sock, id, appInfo);
	} catch (Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : appId/ version / id / name / result");
	}
}

void MSFServer::DoLaunchApplication(int sock, Json::Value& params, std::string id)
{
	REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);

	try {
		if (params["appId"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "id");
			return;
		}
		std::string appID = params["appId"].asString();
		std::string Url = params["url"].asString();

		std::string clientIP = params["clientIp"].asString();
		std::string deviceName = params["deviceName"].isNull()? "SmartView Device": params["deviceName"].asString();
		Json::Value param = params["data"];

		std::string parameter("");
		parameter = param["id"].isNull()? "": param["id"].asString();

		REMOTE_PRINT_DEBUG("[%s][%d] Launch AppIDsssss = %s, Parameter = [%s], clientIP = [%s]\n", __PRETTY_FUNCTION__, __LINE__, appID.c_str(), parameter.c_str(), clientIP.c_str());

		//Check App is installed

		if (NULL != appID.c_str()) {
			int pid = aul_app_get_pid((const char *)appID.c_str());

			REMOTE_PRINT_INFO("BEFORE aul_app_get_pid:[%d] \n", pid);

			REMOTE_PRINT_INFO("[%s][%d] Launch AppID = %s, Parameter = [%s]\n", __PRETTY_FUNCTION__, __LINE__, appID.c_str(), parameter.c_str());

			if (!params["url"].isNull() && appID == "org.tizen.browser")
				callee_pid = REMOTE::App::launch_browser((const char *)Url.c_str());
			else
				callee_pid = REMOTE::App::launch_app((const char *)appID.c_str());

			REMOTE_PRINT_INFO("REMOTE::App::launch_app callee_pid:[%d] \n", callee_pid);

			pid = aul_app_get_pid((const char *)appID.c_str());

			REMOTE_PRINT_INFO("AFTER aul_app_get_pid:[%d] \n", pid);

			if (0 <= callee_pid)
				REMOTE_PRINT_INFO("  Launch remote-server succesfull ");
			else
				REMOTE_PRINT_INFO("  Launch remote-server failed ");

			//Wait for starting
			int count = 0;

			while (count++ < 10) {
				if (aul_app_is_running(appID.c_str()))
					break;

				usleep(500000);
			}
		} else {
			CMSFSend::SendErrorMessage(sock, MSF_NOTFOUND, std::string("Not found error."), std::string(""), id);
			return;
		}

		CMSFSend::SendRespMessage(sock, id, Json::Value(true));
	} catch(Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : appId/ clientIp/ data/ silent/ id/ testmode/ url");
	}
}

void MSFServer::DoStopApplication(int sock, Json::Value& params, std::string id)
{
	REMOTE_PRINT_DEBUG(" DoStopApplication");
	try {
		if (params["appId"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "id");
			return;
		}
		std::string appID = params["appId"].asString();
	//	was_app_info_s widget_info;

		int ret;

		if (aul_app_is_running(appID.c_str())) {
			ret = REMOTE::App::terminate_app(callee_pid);
			if (ret < 0) {
				REMOTE_PRINT_INFO("REMOTE::App::terminate_app FAIL [%d]", ret);
				// resp["result"] = "false";
			} else {
				REMOTE_PRINT_INFO("REMOTE::App::terminate_app SUCCESS");
				// resp["result"] = "true";
			}
		}

		CMSFSend::SendRespMessage(sock, id, Json::Value(true));
	} catch (Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : appId / id / url / result");
	}
}

void MSFServer::DoInstallApplication(int sock, Json::Value& params, std::string id)
{
	try {
		REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);

		if (params["appId"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "appId");
			return;
		}
		std::string appID = params["appId"].asString();

		if (appID != "org.tizen.browser") {
	      		CMSFSend::SendErrorMessage(sock, MSF_NOTFOUND, std::string("Not found error."), std::string(""), id);
      			return ;
	    	}

		REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);
		REMOTE_PRINT_INFO("[INFO] We do NOT support this function 'DoInstallApplication' now. Sorry.");

		CMSFSend::SendRespMessage(sock, id, Json::Value(true));
	} catch (Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : appId / id ");
	}
}

void MSFServer::DoCheckPartyMode(int sock, Json::Value& params, std::string id)
{
	REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);

	try {
		if (params["appId"].isNull()) {
			CMSFSend::SendParamsErrorMessage(sock, id, "appId");
			return;
		}
		std::string appID = params["appId"].asString();
		Json::Value rslt = Json::Value(Json::objectValue);
		rslt["result"] = Json::Value(true);
		rslt["id"] = id;

		REMOTE_PRINT_INFO("D %s\n", __PRETTY_FUNCTION__);
		REMOTE_PRINT_INFO("[INFO] We do NOT support this function 'DoCheckPartyMode' now. Sorry.");

		rslt["partyMode"] = Json::Value(false);
		rslt["running"] = Json::Value(false);

		CMSFSend::SendRespMessage(sock, id, Json::Value(rslt));
	} catch(Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : appId / id / partyMode / running / result");
	}
}

bool MSFServer::checkAppsId(vector<std::string> appIdElement)
{
	if (appIdElement[0].length() > MAX_APPS_ID || (string::npos != appIdElement[0].find_first_not_of("0123456789"))) {
		REMOTE_PRINT_WARN("[%s][%d] Apps Id have to be consist of digit", __PRETTY_FUNCTION__, __LINE__);
		return false;
	}
	return true;
}

bool MSFServer::checkWidgetId(std::vector<std::string> appIdElement)
{
	std::vector<std::string>::iterator it;
	bool flag = true;

	for (it = appIdElement.begin(); it != appIdElement.end(); ++it) {
		unsigned int max = flag? MAX_PROJECT_NAME: MAX_PACKAGE_ID;
		flag = !flag;

		if ((*it).length() > max || std::string::npos != (*it).find_first_of("~!@#$%^&*()_-+=|\'\"[]{}<>?/.,;:`")) {
			REMOTE_PRINT_WARN("[%s][%d] Widget Id have to be consist of digit or character", __PRETTY_FUNCTION__, __LINE__);
			return false;
		}
	}
	return true;
}

bool MSFServer::CheckValidOfappId(std::string id)
{
	if (id.length() > MAX_PROJECT_NAME + MAX_PACKAGE_ID + 1)
		return false;

	int typeOfAppId = REMOTE::String::Split(id, appIdElement, ".");

	bool isValid = false;
	switch (typeOfAppId) {
	case 0:
		isValid = checkAppsId(appIdElement);
		break;
	case 2:
		isValid = checkWidgetId(appIdElement);
		break;
	default:
		isValid = false;
		break;
	}

	REMOTE_PRINT_DEBUG("[%s][%d] AppId's validation is [ %d ] \n", __PRETTY_FUNCTION__, __LINE__, isValid);
	return isValid;
}

void MSFServer::DoACLPairing(int sock, Json::Value& params, std::string id)
{
}

int MSFServer::DecodeBase64(const char *src, int srcLength, char *dst)
{
	int i = 0;
	int offset = 0;
	int dstIndex = 0;

	while (offset < srcLength) {
		unsigned int block = 0;
		int encodingSize = srcLength - offset;
		if (encodingSize > 4)
			encodingSize = 4;

		for (i = 0; i < encodingSize; i++) {
			char val = 0;
			if (src[offset] >= 'A' && src[offset] <= 'Z')
				val = src[offset] - 'A';
			else if (src[offset] >= 'a' && src[offset] <= 'z')
				val = src[offset] - 'a' + 'Z' - 'A' + 1;
			else if (src[offset] >= '0' && src[offset] <= '9')
				val = src[offset] - '0' + ('Z' - 'A' + 1) * 2;
			else if (src[offset] == '+')
				val = 62;
			else if (src[offset] == '/')
				val = 63;

			if (src[offset] != '=') {
				unsigned int decode = 0;
				decode = val;

				decode = decode << (encodingSize - i - 1) * 6;
				block |= decode;
			}
			offset++;
		}

		for (i = 0; i < encodingSize - 1; i++)
			memcpy(&dst[dstIndex++], ((char *)&block) + encodingSize - i - 2, 1);
	}
	return dstIndex;
}

