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

#ifndef _CCDEBUG_REMOTE_H_
#define _CCDEBUG_REMOTE_H_

#include <dlog/dlog.h>
#undef LOG_TAG
#define LOG_TAG "Remote_CM"

#define REMOTE_PRINT_DEBUG(format, args...) LOGD(format"\n", ##args)
#define REMOTE_PRINT_INFO(format, args...) LOGI(format"\n", ##args)
#define REMOTE_PRINT_WARN(format, args...) LOGW(format"\n", ##args)
#define REMOTE_PRINT_MAJOR(format, args...) LOGE(format"\n", ##args)
#define REMOTE_PRINT_FATAL(format, args...) LOGE(format"\n", ##args)

#define REMOTE_SYSPRINT_DEBUG(format, args...) SLOGD(format"\n", ##args)
#define REMOTE_SYSPRINT_INFO(format, args...) SLOGI(format"\n", ##args)
#define REMOTE_SYSPRINT_WARN(format, args...) SLOGW(format"\n", ##args)
#define REMOTE_SYSPRINT_MAJOR(format, args...) SLOGE(format"\n", ##args)
#define REMOTE_SYSPRINT_FATAL(format, args...) SLOGE(format"\n", ##args)

//No Need
#define REMOTERCR_PRINT_DEBUG(format, args...) LOGD(format"\n", ##args)
#define REMOTERCR_PRINT_INFO(format, args...) LOGI(format"\n", ##args)
#define REMOTERCR_PRINT_WARN(format, args...) LOGW(format"\n",  ##args)
#define REMOTERCR_PRINT_MAJOR(format, args...) LOGE(format"\n", ##args)
#define REMOTERCR_PRINT_FATAL(format, args...) LOGE(format"\n", ##args)

//No Need
#define REMOTECM_PRINT_DEBUG(format, args...) LOGD(format"\n", ##args)
#define REMOTECM_PRINT_INFO(format, args...) LOGI(format"\n", ##args)
#define REMOTECM_PRINT_WARN(format, args...) LOGW(format"\n", ##args)
#define REMOTECM_PRINT_MAJOR(format, args...) LOGE(format"\n", ##args)
#define REMOTECM_PRINT_FATAL(format, args...) LOGE(format"\n", ##args)

#define __WDC_LOG_FUNC_START__ LOGV("Enter")
#define __WDC_LOG_FUNC_END__ LOGV("Quit")

/*

#define REMOTE_PRINT_DEBUG(format, args...) 
#define REMOTE_PRINT_INFO(format, args...) 
#define REMOTE_PRINT_WARN(format, args...) 
#define REMOTE_PRINT_MAJOR(format, args...)
#define REMOTE_PRINT_FATAL(format, args...) 

#define REMOTERCR_PRINT_DEBUG(format, args...) 
#define REMOTERCR_PRINT_INFO(format, args...) 
#define REMOTERCR_PRINT_WARN(format, args...) 
#define REMOTERCR_PRINT_MAJOR(format, args...)
#define REMOTERCR_PRINT_FATAL(format, args...) 

#define REMOTECM_PRINT_DEBUG(format, args...) 
#define REMOTECM_PRINT_INFO(format, args...) 
#define REMOTECM_PRINT_WARN(format, args...) 
#define REMOTECM_PRINT_MAJOR(format, args...)
#define REMOTECM_PRINT_FATAL(format, args...) 

#define __WDC_LOG_FUNC_START__
#define __WDC_LOG_FUNC_END__

*/

#endif //_CCDEBUG_REMOTE_H_

