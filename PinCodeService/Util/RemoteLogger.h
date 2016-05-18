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


#ifndef _RM_KPILOG_H_
#define _RM_KPILOG_H_

#include <string>

using namespace std;

typedef enum _eMSFBIType {
	eMSFBI_connect_installedapp = 1,
	eMSFBI_connect_cloudapp,
	eMSFBI_disconnect_installedapp,
	eMSFBI_disconnect_cloudapp,
	eMSFBI_show_installpage,
	eMSFBI_dial,
	eMSFBI_error,
	eMSFBI_connect_mediareceiver,
	eMSFBI_disconnect_mediareceiver,
	eMSFBI_dial_device_allow,
	eMSFBI_dial_device_deny,
	eMSFBI_MAX
} eMSFBIType;

typedef struct _MSFBIconf {
	std::string Name;
	std::string Type;
	int iLogLevel;
	std::string strLoglevel;
} MSFBIconf;

#define QUEUE_MAX 30
#define EXPIRATION 60
#define THRESHOLD 0
#define LOGLEVEL 10
#define EVENT_LOGLEVEL 5
#define INFO_LINK_OPERATIING_SERVER 0   //dummy

class CRemoteLogger {
public:
	MSFBIconf m_stMSFBIConf[eMSFBI_MAX];

	CRemoteLogger(void);
	~CRemoteLogger(void);

	static CRemoteLogger* Get(void);
	bool AddBILog(eMSFBIType eType, std::string data, std::string err);
	bool AddEventLog(const std::string sEventName, const std::string sCategory, const std::string sDesc);

private:
  static CRemoteLogger *s_pInstance;

  bool m_Create(void);
  std::string m_pServerURL;

  bool m_GetFWVersion(void);
  std::string m_strFWVersion;
};

#endif /* _RM_KPILOG_H_ */
