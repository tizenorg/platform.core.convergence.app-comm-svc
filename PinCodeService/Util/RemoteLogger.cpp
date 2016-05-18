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

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <RemoteLogger.h>
#include <CCDebugRemote.h>
#include <vconf.h>
#include <system_info.h>
#include "MSFVconfDefine.h"


CRemoteLogger* CRemoteLogger::s_pInstance = NULL;

CRemoteLogger::CRemoteLogger(void)
{
	m_stMSFBIConf[eMSFBI_connect_installedapp].Name = std::string("CONNECT_INSTALLEDAPP");
	m_stMSFBIConf[eMSFBI_connect_installedapp].Type = std::string("EV001");

	m_stMSFBIConf[eMSFBI_connect_cloudapp].Name = std::string("CONNECT_CLOUDAPP");
	m_stMSFBIConf[eMSFBI_connect_cloudapp].Type = std::string("EV002");

	m_stMSFBIConf[eMSFBI_disconnect_installedapp].Name = std::string("DISCONNECT_INSTALLED");
	m_stMSFBIConf[eMSFBI_disconnect_installedapp].Type = std::string("EV003");

	m_stMSFBIConf[eMSFBI_disconnect_cloudapp].Name = std::string("DISCONNECT_CLOUDAPP");
	m_stMSFBIConf[eMSFBI_disconnect_cloudapp].Type = std::string("EV004");

	m_stMSFBIConf[eMSFBI_show_installpage].Name = std::string("INSTALL_APP");
	m_stMSFBIConf[eMSFBI_show_installpage].Type = std::string("EV005");

	m_stMSFBIConf[eMSFBI_dial].Name = std::string("DIAL_CONNECT");
	m_stMSFBIConf[eMSFBI_dial].Type = std::string("EV006");

	m_stMSFBIConf[eMSFBI_error].Name = std::string("ERROR_LOG");
	m_stMSFBIConf[eMSFBI_error].Type = std::string("EV007");

	m_stMSFBIConf[eMSFBI_connect_mediareceiver].Name = std::string("CONNECT_MEDIARECEIVE");
	m_stMSFBIConf[eMSFBI_connect_mediareceiver].Type = std::string("EV008");

	m_stMSFBIConf[eMSFBI_disconnect_mediareceiver].Name = std::string("DISCONNECT_MEDIARECE");
	m_stMSFBIConf[eMSFBI_disconnect_mediareceiver].Type = std::string("EV009");

	m_stMSFBIConf[eMSFBI_dial_device_allow].Name = std::string("DIAL_DEVICE_ALLOW");
	m_stMSFBIConf[eMSFBI_dial_device_allow].Type = std::string("EV010");

	m_stMSFBIConf[eMSFBI_dial_device_deny].Name = std::string("DIAL_DEVICE_DENY");
	m_stMSFBIConf[eMSFBI_dial_device_deny].Type = std::string("EV011");
}

CRemoteLogger::~CRemoteLogger(void)
{
	if (s_pInstance != NULL) {
		delete s_pInstance;
		s_pInstance = NULL;
	}
}

CRemoteLogger* CRemoteLogger::Get(void)
{
	if (NULL == s_pInstance)
		s_pInstance = new CRemoteLogger();

	return s_pInstance;
}

bool CRemoteLogger::AddBILog(eMSFBIType eType, std::string data1, std::string data2)
{
	std::string sDescription;
	switch (eType) {
	case eMSFBI_connect_installedapp:
	case eMSFBI_disconnect_installedapp:
	case eMSFBI_show_installpage:
		sDescription.append("{ai=");
		sDescription.append(data1);
		sDescription.append(";er=");
		sDescription.append(data2);
		sDescription.append("}");
		break;
	case eMSFBI_connect_cloudapp:
	case eMSFBI_disconnect_cloudapp:
		sDescription.append("{au=");
		sDescription.append(data1);
		sDescription.append(";er=");
		sDescription.append(data2);
		sDescription.append("}");
		break;
	case eMSFBI_dial:
		sDescription.append("{ai=");
		sDescription.append(data1);
		sDescription.append("}");
		break;
	case eMSFBI_error:
		sDescription.append("{er=");
		sDescription.append(data1);
		sDescription.append("}");
		break;
	case eMSFBI_connect_mediareceiver:
	case eMSFBI_dial_device_allow:
	case eMSFBI_dial_device_deny:
		sDescription.append("{ty=");
		sDescription.append(data1);
		sDescription.append("}");
		break;
	case eMSFBI_disconnect_mediareceiver:
		sDescription.append("{}");
		break;
	default:
		return false;
	}

	return AddEventLog(m_stMSFBIConf[eType].Name, m_stMSFBIConf[eType].Type, sDescription);
}

bool CRemoteLogger::AddEventLog(const std::string sEventName, const std::string sCategory, const std::string sDesc)
{
	if (false == m_Create()) {
		REMOTE_PRINT_FATAL("cannot create logger instance");
		return false;
	}

    time_t timer;
    time(&timer);
    char strTime[32] = {0, };

    tm p;
    localtime_r(&timer, &p);
    p.tm_year = p.tm_year + 1900;
    p.tm_mon = p.tm_mon + 1;

    snprintf(strTime, 32, "%04d-%02d-%02dT%02d:%02d:%02d+%02d%02d", p.tm_year, p.tm_mon, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec, 0, 0);

    std::string strTimeStamp(strTime);

    m_GetFWVersion();
    REMOTE_PRINT_DEBUG("KPI Log [%s] (%s)", sEventName.c_str(), sDesc.c_str());

	return true;
}

bool CRemoteLogger::m_Create(void)
{
	for (int i = 1; i < eMSFBI_MAX; i++) {
		m_stMSFBIConf[i].strLoglevel = "5";
		m_stMSFBIConf[i].iLogLevel = EVENT_LOGLEVEL;
	}

	//add events
	for (int i = 1; i < eMSFBI_MAX; i++) {
	}

	char *strPSID = NULL;
	strPSID = vconf_get_str(VCONFKEY_REMOTE_SERVER_DEVICE_PSID);
	REMOTE_PRINT_DEBUG("psid : %s", strPSID);
	free(strPSID);

	return true;
}

bool CRemoteLogger::m_GetFWVersion(void)
{
	char *strBuildVer = NULL;

	if (strBuildVer != NULL) {
		m_strFWVersion.assign(strBuildVer);
		free(strBuildVer);
		strBuildVer = NULL;
	} else {
		if (strBuildVer == NULL) {
			REMOTE_PRINT_DEBUG("ERR: no build version");
			return false;
		}
	}
	return true;
}

