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

#include "MSFSend.h"
#include "CCDebugRemote.h"

#include <stdio.h>
#include <sys/socket.h>

#undef LOG_TAG
#define LOG_TAG "Remote_Daemon"

void CMSFSend::SendRespMessage(int sock, std::string id, Json::Value result)
{
	try {
		Json::Value resp = Json::Value(Json::objectValue);
		resp["id"] = id;
		resp["result"] = result;
		Json::FastWriter writer;
		Send(sock, writer.write(resp));
	} catch (Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : result/ id ");
	}
}

void CMSFSend::SendParamsErrorMessage(int sock, std::string id, std::string parameter)
{
	try {
		SendErrorMessage(sock, MSF_INVALID_PARAMS, std::string("Parameter not found."), parameter, Json::Value(id));
	} catch(Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception :  id");
	}
}

void CMSFSend::SendErrorMessage(int sock, int errorCode, std::string errorMessage, std::string errorData, Json::Value id)
{
	REMOTE_PRINT_DEBUG("[%s]%s", __PRETTY_FUNCTION__, errorMessage.c_str());
	try {
		Json::Value errorJson = Json::Value(Json::objectValue);
		errorJson["error"] = Json::Value(Json::objectValue);
		errorJson["error"]["code"] = Json::Value(errorCode);
		errorJson["error"]["message"] = Json::Value(errorMessage);
		errorJson["error"]["data"] = Json::Value(errorData);
		errorJson["id"] = id;
		Json::FastWriter writer;
		Send(sock, writer.write(errorJson));
	} catch(Json::Exception& e) {
		REMOTE_PRINT_WARN("JSON exception : error/ code / message/ data / id");
	}
}

void CMSFSend::Send(int sock, std::string data)
{
	char length[64] = "";
	snprintf(length, 63, "%d,", data.length());

	std::string sendmsg = std::string(length) + data;
	int sendmsgLength = sendmsg.length();
	const char *sendmsg_ptr = sendmsg.c_str();

	REMOTE_PRINT_INFO("[%s][%d] \n######### %s\n", __PRETTY_FUNCTION__, __LINE__, sendmsg.c_str());

	int sent = 0;
	do {
		sent += send(sock, sendmsg_ptr + sent, sendmsgLength - sent, 0);
	} while (sent < sendmsgLength);
	//sendmsg_ptr = null;
}
