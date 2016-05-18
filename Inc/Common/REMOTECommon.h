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
 * @file        REMOTECommon.h        
 * @brief       This file consists of all necessary #define and necessary typedefs borrowed 
                from other Orsay projects (e.g. from BP_HAL, AP_Calla etc.) 
 * @created on: 2013.10.30
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */

#ifndef REMOTECOMMON_H_
#define REMOTECOMMON_H_

//#define NULL 0

#include <assert.h>
#include <stddef.h>

#define ASSERT(t) assert(t)

#ifndef INT_APP_ASSERT
#define INT_APP_ASSERT(VAL)        \
    {            \
        bool __ret = (VAL)? true: false;    \
        if (!__ret) {     \
            ASSERT(VAL);        \
            return 0;    \
        }            \
    }
#endif

//Borrowed from BP_HAL - shivaram.y:

#endif /* REMOTECOMMON_H_ */
