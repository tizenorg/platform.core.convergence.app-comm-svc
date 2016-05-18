
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
 * @file        REMOTEApp.h
 * @brief       App class provides functions that manage the App.
 * @created on: 2016.05.16
 * @author: Hongkuk Son (hongkuk.son@samsung.com)
 * @reviewer: Jihoon Park (jpark0@samsung.com)
 */

#ifndef REMOTEAPP_H_
#define REMOTEAPP_H_

#undef LOG_TAG
#define LOG_TAG "rep_WAS"
#define PRINT_INFO(format, args...) LOGI(format"\n", ##args)

#include <stdio.h>
#include <stdlib.h>
#include <dlog/dlog.h>
#include "REMOTECommon.h"

namespace REMOTE {

class App {
private:
    App(void) {}

public:
	static int launch_app(const char *appid);		// launch application
	static int launch_browser(const char *url);	// launch org.tizen.browser with given url
	static int terminate_app(int pid); 		// terminate application
	static int get_current_showing_app(int *pid);
	static int get_current_showing_app(char **appid);
	static int get_app_status(int pid, int *status);
};

}

#endif /* REMOTEAPP_H_ */
