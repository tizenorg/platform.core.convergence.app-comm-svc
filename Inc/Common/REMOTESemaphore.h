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
 * @file        REMOTESemaphore.h   
 * @brief       Semaphore class provides functions that create and manage Semaphore. 
 * @created on: 2013.08.31
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */

#ifndef REMOTESEMAPHORE_H_
#define REMOTESEMAPHORE_H_

#include <errno.h>
#include "REMOTECommon.h"

struct PTSemaphore;

namespace REMOTE {
	class Semaphore {
	public:
		Semaphore(void) { m_id = NULL; }
		virtual ~Semaphore(void) {}

		//! Create the instance
		bool Create(int count);
		//! Check if the instance was created
		bool FlagCreate(void) { return m_id != NULL; }
		//! Destroy the instance
		virtual void Destroy(void);

		//! Decreases the counter of REMOTE::Semaphore.
		void Take(void);
		//! Tries to decrease the counter of REMOTE::Semaphore.
		bool Try(unsigned long msec = 0);
		//! Increases the counter of REMOTE::Semaphore.
		void Give(void);

	private:
		PTSemaphore *m_id;
	};
}

#endif /* REMOTESEMAPHORE_H_ */
