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
 * @file        REMOTEMutex.h   
 * @brief       Mutex class provides functions that create and manage mutex. 
 * @created on: 2013.08.31
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */

#ifndef REMOTEMUTEX_H_
#define REMOTEMUTEX_H_

#include "REMOTECommon.h"
#include "REMOTESemaphore.h"
#include "pthread.h"
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>

namespace REMOTE {
	class Mutex {
	public:
		//! The constructor of REMOTE::Mutex.
		Mutex(void) { m_threadId = 0; }
		//! The destructor of REMOTE::Mutex.
		virtual ~Mutex(void) {}

		//! Create the instance
		bool Create(void);
		//! Check if the instance was created
		bool FlagCreate(void) { return m_sem.FlagCreate(); }
		//! Destroy the instance
		virtual void Destroy(void);

		//! Locks the created REMOTE::Mutex object.
		void Lock();
		//! Tries to lock the REMOTE::Mutex object.
		bool Try(unsigned long msec = 0);
		//! Unlocks the REMOTE::Mutex object.
		bool Unlock(void);

	private:
		REMOTE::Semaphore m_sem;
		int m_threadId;
		int m_count;
	};
}

#endif /* REMOTEMUTEX_H_ */
