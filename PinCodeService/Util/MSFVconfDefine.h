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

#ifndef _MSFDEFINE_H_
#define _MSFDEFINE_H_

#include <vconf.h>           // Tizen vconf Header
#include <vconf-keys.h>

#define VCONFKEY_REMOTE_SERVER_COUNTRY_ID "db/comss/countrycode"
#define VCONFKEY_REMOTE_SERVER_DUID  "db/comss/duid"
#define VCONFKEY_REMOTE_SERVER_MODEL_ID "db/comss/modelid"
#define VCONFKEY_REMOTE_SERVER_DEVICE_NAME "db/menu/network/devicename/tv_name"
#define VCONFKEY_REMOTE_SERVER_INFOLINK_VER "db/comss/infolinkversion"
#define VCONFKEY_REMOTE_SERVER_DEVELOPER_MODE "db/sdk/develop/mode"
#define VCONFKEY_REMOTE_SERVER_DEVELOPER_IP "db/sdk/develop/ip"
#define VCONFKEY_REMOTE_SERVER_ATOKEN "db/comss/atoken"
#define VCONFKEY_REMOTE_SERVER_HUB_TERMS "db/menu/smart_hub/homedatacontrol/defaultdisclaimeragree"
#define VCONFKEY_REMOTE_SERVER_MLS_STATE_DB "db/mls/mls_state"
#define VCONFKEY_REMOTE_SERVER_DEVICE_PSID "db/adagent/psid"

#endif	/* _MSFDEFINE_H_ */
