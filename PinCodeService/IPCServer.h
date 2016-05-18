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

/** 
 * @file	IPCServer.h
 * @brief	Create socket and Send, Recv by socket
 *
 * IPCServer Class definition file
 * Copyright 2013 by Samsung Electronics, Inc.,
 */

#ifndef _IPCSERVER_H_
#define _IPCSERVER_H_

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
#include "REMOTEThread.h"
//#include "IInfoCallback.h"
#include "json/json.h"
#include "MSFVconfDefine.h"
#include "wifi.h"

#define PCS_REMOTE_SOCKET_PATH	"/tmp/pcs"

#undef LOG_TAG
#define LOG_TAG "Remote_Daemon"


/**
 * @class	IPCServer
 * @brief	IPCServer Class use BSD socket. Make socket for TV connection and Thread for recv
 *
 * @see	Packet
 
 * Copyright 2013 by Samsung Electronics, Inc.,
 */
class IPCServer : public REMOTE::Thread {
private:

protected:
	int m_socket;
	pthread_mutex_t mutex;
	std::string PinCodeToDisplay;
//	IInfoCallback* callback;

protected:
	void t_Main(void);

	bool PacketParse(FILE *file, int sock);
	std::string MakeInformation();

public:
	/**
	 * @fn			IPCServer(void);
	 * @brief		Constructor of IPCServer
	 * @exception	N/A
	 */
	IPCServer(void);
	/**
	 * @fn			IPCServer(void);
	 * @brief		Destructor of IPCServer
	 * @exception	N/A
	 */
	virtual ~IPCServer(void);

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

	std::string GetPinCode();
	std::string GetPINFromAppID(std::string appID);
	bool GetPanelResolution(int &width, int &height);
	static int cb(int msgType, void *cookie, void *userdata);
	void UpdateMITNode();
	int IsAgreedSmartHubTerms(void);
	//void SetCallback(IInfoCallback* cb);
private:
	void ReadBTMAC(char *);
	void ReadWifiMAC(char *);
	void Read_p2p_MAC(char *);
	char make_p2p_mac(char);
	void ConvertCapitalMac(char *);
	//void VDCurlMessageSend(void);
	//static void getMSFAppInfo_cb(keynode_t *key, void* data);
};

#endif	/* _IPCSERVER_H_ */
