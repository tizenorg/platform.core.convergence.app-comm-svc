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

#ifndef _MSFSERVER_H_
#define _MSFSERVER_H_

#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <json/json.h>
#include "REMOTEMutex.h"

#undef LOG_TAG
#define LOG_TAG "Remote_Daemon"

#define MSF_SOCKET_PATH	"/tmp/msf"
#define MSF_BUFFER_SIZE	(256)

#define PARTYMODE_APPID "org.volt.partymode"
#define CLOUD_APPID "3201412000694"

#define MAX_PROJECT_NAME 10
#define MAX_PACKAGE_ID 50
#define MAX_APPS_ID 13

struct MSFApplicationInfo {
	std::string id;
	std::string version;
	std::string name;
};

struct MSF_WIDGET_INFO {
	std::string id;
	std::string name;
	std::string version;
	std::string runtitle;
};

class MSFServer {
protected:
	int m_socket;
	pthread_t recvThread;
	REMOTE::Mutex launchLock;

	std::map<std::string/* App id */, struct MSF_WIDGET_INFO> widgetList;
	std::map<std::string/* App name */, std::string/* App id */> widgetNameMap;
	std::vector<std::string> appIdElement;

	int DecodeBase64(const char *src, int srcLength, char *dst);
	bool CheckValidOfappId(std::string id);
	bool checkAppsId(std::vector<std::string> appIdElement);
	bool checkWidgetId(std::vector<std::string> appIdElement);

	bool PacketParse(FILE *file, int sock);
	void ParseJSON(int sock, std::string contents);
	int StartRecv(void);

	void DoGetApplication(int sock, Json::Value& params, std::string id);
	void DoLaunchApplication(int sock, Json::Value& params, std::string id);
	void DoStopApplication(int sock, Json::Value& params, std::string id);
	void DoInstallApplication(int sock, Json::Value& params, std::string id);
	void DoACLPairing(int sock, Json::Value& params, std::string id);

public:
	/**
	 * @fn			MSFServer(void);
	 * @brief		Constructor of NGameManager
	 * @exception	N/A
	 */
	MSFServer(void);
	/**
	 * @fn			~MSFServer(void);
	 * @brief		Destructor of NGameManager
	 * @exception	N/A
	 */
	virtual ~MSFServer(void);

	/**
	 * @fn			int Init();
	 * @brief		Initialize socket and regist callback function.
	 * @param[in]	parser	Callback function to process received packet.
	 * @return		int		If return value is over 0, Initialize is success. 
	 * @exception	N/A
	 */
	int Init(void);

	/**
	 * @fn			void Final(void);
	 * @brief		Close and Release socket.
	 * @return		void
	 * @exception	N/A
	 */
	void Final(void);

	/**
	 * @fn			static void* SocketThread(void* arg);
	 * @brief		Thread function for iPhone/Linux
	 * @return		void*
	 * @exception	N/A
	 */
	static void* SocketThread(void *arg);

	/**
	 * @fn			void SocketProcess(void);
	 * @brief		Actually SocketProcess method work receive from server. It's called by SocketThread method.
	 * @return		void*
	 * @exception	N/A
	 */
	void SocketProcess(void);

	void DoCheckPartyMode(int sock, Json::Value& params, std::string id);
};

#endif	/* _MSFSERVER_H_ */
