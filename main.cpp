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

/*Product: Samsung NService Framework (remote-server)
 * Version: 1.0
 *
 * Copyright 2013-2014 SWC, Samsung Electronics, Inc.
 * All rights reserved.
 */

/**
 * @file 	main.cpp
 * @brief	This file is resposible to run the convergence manager as deamon.
                and initialize the RCR and convergence manager
 * @created on: 2013.10.30
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */


#include <CCDebugRemote.h>

#include "IPCServer.h"
#include "MSFServer.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <glib.h>
#include <systemd/sd-daemon.h>
#include <sys/stat.h>
#include <wait.h>
#include <vconf.h>
#include "net_connection.h"

extern "C" {
    #include <argos.h>
}

#define config_path_nservice "/opt/usr/apps/remote-server"
#define uri_nservice         "nservice/"

#define NOTIFY_TIME 15000	//watchdog notify time

#undef LOG_TAG
#define LOG_TAG "Remote_Daemon"


static GMainLoop*    mainloop = NULL;
IPCServer* ipc_server = NULL;
MSFServer* msf_server = NULL;
const char *service_name = "remote-server";

gboolean watchdog_notify_callback(gpointer data)
{
	int ret = 0;
	ret = aw_notify();

	if(ret < 0)
	{
		REMOTE_PRINT_WARN("smart_deadlock_watchdog_notify failed return = %d", ret);
	}


	REMOTE_PRINT_WARN("[sd_notify debug] called sd_notify() : ret = %d : Proc : %s(%d)", ret, service_name, (unsigned long)getpid());

	return TRUE;
}

static void CheckWifiSecurityEncryption(connection_h connection_info)
{
	//Check_Result (Wifi Check for Omission OK : 0, Not OK : 1)
	//int Check_Result = 1;

	REMOTE_PRINT_FATAL("CheckWifiSecurityEncryption START");

	connection_h connection = connection_info;
	connection_type_e connection_type;
	connection_profile_h connection_profile;
	connection_wifi_security_type_e connection_wifi_security_type;
	connection_wifi_encryption_type_e connection_wifi_encryption_type;

	if(connection == NULL)
	{
		REMOTE_PRINT_FATAL("Failed to get Connection Profile");
		//pCMInstance->SetACLOmissionResult(Check_Result);
		return;
	}

	// Check WIFI & WPA2 & AES (If all those are applied to TV, Allow Omission Access Popup)
	if(connection_get_type(connection, &connection_type) == CONNECTION_ERROR_NONE)
	{
		if(connection_type == CONNECTION_TYPE_WIFI)
		{
			if(connection_get_current_profile(connection, &connection_profile) != CONNECTION_ERROR_NONE)
			{
				REMOTE_PRINT_FATAL("Failed to get Connection Profile");
				//pCMInstance->SetACLOmissionResult(Check_Result);
				return;
			}

			if(connection_profile_get_wifi_security_type(connection_profile, &connection_wifi_security_type) != CONNECTION_ERROR_NONE)
			{
				REMOTE_PRINT_DEBUG("Failed to get Wifi Security Type");
			}
			else if (connection_profile_get_wifi_encryption_type(connection_profile, &connection_wifi_encryption_type) != CONNECTION_ERROR_NONE)
			{
				REMOTE_PRINT_DEBUG("Failed to get Wifi Encryption Type");
			}
			else if ( (connection_wifi_security_type == CONNECTION_WIFI_SECURITY_TYPE_WPA_PSK) &
				( (connection_wifi_encryption_type == CONNECTION_WIFI_ENCRYPTION_TYPE_AES) || (connection_wifi_encryption_type == CONNECTION_WIFI_ENCRYPTION_TYPE_TKIP_AES_MIXED)))
			{
				//Check_Result = 0;
				REMOTE_PRINT_DEBUG("Wifi Check for Omission is OK");
				//pCMInstance->SetACLOmissionResult(Check_Result);
				return;
			}
			else
			{
				REMOTE_PRINT_DEBUG("Not Allow Ommission Of Access Popup : Because of Security type & Encryption type");
			}
		}
	}
	else
	{
		REMOTE_PRINT_DEBUG("Failed to get Connection Type");
	}
	//pCMInstance->SetACLOmissionResult(Check_Result);
	REMOTE_PRINT_FATAL("Wifi Check for Omission is Not OK");
}

void InitUpnpDevices()
{
	//Init IPC server
	ipc_server = new IPCServer();
	ipc_server->Init();

	msf_server = new MSFServer();
	msf_server->Init();
}

static void Setup_CMDaemon()
{
	InitUpnpDevices();

	mainloop = g_main_loop_new(NULL, FALSE);
}

void getTvName_cb(keynode_t *key, void* data)
{
	ipc_server->UpdateMITNode(); // update Node Server When TV Name is changed.
}

void NetworkIPChanged_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	if(ipv4_address != 0 && (strlen(ipv4_address) > 7))
	{
		REMOTE_PRINT_DEBUG("UpdateMITNode request, IP_address : [%s]", ipv4_address);
		ipc_server->UpdateMITNode(); //update Node Server when IP is changed.

		CheckWifiSecurityEncryption(user_data);
	}
	else
	{
		REMOTE_PRINT_DEBUG("There is no IP address");
		//pCMInstance->SetACLOmissionResult(1);	//Check WIFI for omission is 'Not OK'(1)
	}
}

int main(void)
{
	REMOTE_PRINT_MAJOR("remote server main");
	try
	{
		Setup_CMDaemon();

		vconf_notify_key_changed("db/menu/network/devicename/tv_name", getTvName_cb, NULL);

		if(mainloop != NULL)
		{
			connection_h connection = NULL;
			int ret = -1;
			ret = connection_create(&connection);
			if(ret == CONNECTION_ERROR_NONE)
			{
//				connection_set_type_changed_cb(connection, NetworkTypeChanged_cb, NULL);
				CheckWifiSecurityEncryption(connection);
				connection_set_ip_address_changed_cb(connection, NetworkIPChanged_cb, connection);
			}
			else
			{
				REMOTE_PRINT_DEBUG("Fail to create connection_set_type_changed_cb\n");
			}
			//REMOTE_PRINT_DEBUG("Start Convergence Manager!");
			aw_register(60);

			g_timeout_add(NOTIFY_TIME, watchdog_notify_callback, mainloop);


			// Run GMainLoop
			REMOTE_PRINT_DEBUG("remote-server mainloop Start");
			g_main_loop_run(mainloop);
			g_main_loop_unref(mainloop);
			connection_destroy(connection);
		}
		else
		{
			REMOTE_PRINT_WARN("Fail to start Convergence Manager!");
		}

		vconf_ignore_key_changed("db/menu/network/devicename/tv_name", getTvName_cb);
	}
	catch(...)
	{
	}
	return 0;
}
