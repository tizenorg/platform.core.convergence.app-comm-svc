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

#include "REMOTEApp.h"
#include <aul.h>
#include <aul_svc.h> // for using aul_svc_set_uri()
#include <bundle.h>
#include <tzplatform_config.h>
#include "CCDebugRemote.h"

int _iter_visible_cb(const aul_app_info *info, void *data)
{
	int ret;
	int *pid = (int *)data;

	ret = aul_app_get_status_bypid_for_uid(info->pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (STATUS_VISIBLE == ret) {
		PRINT_INFO("[%s][%d] STATUS_VISIBLE [%d]", __PRETTY_FUNCTION__, __LINE__, info->pid);

		*pid = info->pid;

		return -1;
	}

	return 0;
}

int REMOTE::App::launch_app(const char *appid)
{
	// if SUCCESS,	return callee's pid
	// if FAIL,	return negative value

	PRINT_INFO("[FUNC] rep_launch_app");

	int ret = -1;

	bundle *pBundle = bundle_create();

	if (BUNDLE_ERROR_NONE != bundle_add_str(pBundle, "AppId", appid)) {
		PRINT_INFO("[%s][%d] bundle_add_str FAIL", __PRETTY_FUNCTION__, __LINE__);
		bundle_free(pBundle);

		return -1;
	}

	if (aul_app_is_running_for_uid(appid, tzplatform_getuid(TZ_SYS_DEFAULT_USER)))
		ret = aul_resume_app_for_uid(appid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));
	else
		ret = aul_launch_app_for_uid(appid, pBundle, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (ret < 0) {
		PRINT_INFO("[%s][%d] aul_launch_app FAIL [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

		return ret;
	}

	if (BUNDLE_ERROR_NONE != bundle_free(pBundle)) { // free bundle
		PRINT_INFO("[%s][%d] bundle_free FAIL", __PRETTY_FUNCTION__, __LINE__);

		return -1;
	}

	PRINT_INFO("[%s][%d] rep_launch_app SUCCESS [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

	return ret;
}

int REMOTE::App::launch_browser(const char *url)
{
	// if SUCCESS,	return callee's pid
	// if FAIL,	return negative value

	PRINT_INFO("[FUNC] rep_launch_browser");

	int ret = -1;

	bundle *pBundle = bundle_create();

	if (BUNDLE_ERROR_NONE != bundle_add_str(pBundle, "AppId", "org.tizen.browser")) {
		PRINT_INFO("[%s][%d] bundle_add_str FAIL", __PRETTY_FUNCTION__, __LINE__);
		bundle_free(pBundle);

		return -1;
	}

	aul_svc_set_uri(pBundle, url); // url setting API

	if (aul_app_is_running_for_uid("org.tizen.browser", tzplatform_getuid(TZ_SYS_DEFAULT_USER)))
		ret = aul_resume_app_for_uid("org.tizen.browser", tzplatform_getuid(TZ_SYS_DEFAULT_USER));
	else
		ret = aul_launch_app_for_uid("org.tizen.browser", pBundle, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (ret < 0) {
		PRINT_INFO("[%s][%d] aul_launch_app FAIL [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

		return ret;
	}

	if (BUNDLE_ERROR_NONE != bundle_free(pBundle)) { // free bundle
		PRINT_INFO("[%s][%d] bundle_free FAIL", __PRETTY_FUNCTION__, __LINE__);

		return -1;
	}

	PRINT_INFO("[%s][%d] rep_launch_app SUCCESS [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

	return ret;
}

int REMOTE::App::terminate_app(int pid)
{
	// current state: CREATE or PAUSED
	// if SUCCESS,	return 0
	// if FAIL,	return negative value

	PRINT_INFO("[FUNC] rep_terminate_app");

	int ret;

	if (pid < 0) {
		return -1;
	} else {
		ret = aul_terminate_pid_for_uid(pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

		if (ret < 0) {
			PRINT_INFO("[%s][%d] aul_terminate_pid FAIL [%d], PID [%d]", __PRETTY_FUNCTION__, __LINE__, ret, pid);

			return ret;
		}
	}

	PRINT_INFO("[%s][%d] rep_terminate_app SUCCESS [%d], PID [%d]", __PRETTY_FUNCTION__, __LINE__, ret, pid);

	return ret;
}

int REMOTE::App::get_current_showing_app(int *pid)
{
	PRINT_INFO("[FUNC] rep_get_current_showing_app [%d]", *pid);

	int ret;

	ret = aul_app_get_running_app_info_for_uid(_iter_visible_cb, pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (ret < 0) {
		PRINT_INFO("[%s][%d] FAIL [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

		return ret;
	}

	PRINT_INFO("[%s][%d] SUCCESS [%d], pid:[%d]", __PRETTY_FUNCTION__, __LINE__, ret, *pid);

	return ret;
}

int REMOTE::App::get_current_showing_app(char **appid)
{
	PRINT_INFO("[FUNC] rep_get_current_showing_app");

	int ret;
	int *pid = NULL;

	ret = aul_app_get_running_app_info_for_uid(_iter_visible_cb, pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (ret < 0) {
		PRINT_INFO("[%s][%d] FAIL [%d]", __PRETTY_FUNCTION__, __LINE__, ret);

		return ret;
	}

	PRINT_INFO("[%s][%d] SUCCESS [%d], pid:[%d]", __PRETTY_FUNCTION__, __LINE__, ret, *pid);

	ret = aul_app_get_appid_bypid_for_uid(*pid, *appid, sizeof(*appid), tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	return ret;
}

int REMOTE::App::get_app_status(int pid, int *status)
{
	int ret;

	ret = aul_app_get_status_bypid_for_uid(pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (ret < 0) {
		return -1;
	} else {
		*status = ret;

		return 0;
	}
}

