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


#ifndef _MSF_SEND_H_
#define _MSF_SEND_H_

#include <string>
#include <json/json.h>
#include "json/writer.h"

#define MSF_NOTFOUND (-4004)
#define MSF_ALREADY_INSTALLED (-4009)
#define MSF_NOTRUNNING (-4444)
#define MSF_UNAUTHORIZED (-4001)
#define MSF_FAIL (-4444)
#define MSF_INVALID_PARAMS	(-32602)
#define MSF_JSON_ERROR (-32700)
#define MLS_NOT_SUPPORTED (-4010)

class CMSFSend {
public:
	static void SendRespMessage(int sock, std::string id, Json::Value result);
	static void SendParamsErrorMessage(int sock, std::string id, std::string parameter);
	static void SendErrorMessage(int sock, int errorCode, std::string errorMessage, std::string errorData, Json::Value id);
	static void Send(int sock, std::string data);
};
#endif // _MSF_SEND_H_
