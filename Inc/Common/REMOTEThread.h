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
 * @file        REMOTEThread.h   
 * @brief       Theard class provides functions that create and manage the Threads. 
 * @created on: 2013.08.31
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */

#ifndef REMOTETHREAD_H_
#define REMOTETHREAD_H_
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>

#include "REMOTECommon.h"

namespace REMOTE {
	class Thread {
	public:
		Thread();
		virtual ~Thread(void);

		//! Create the instance
		bool Create(unsigned long stackSize = CONFIG_STACK_SIZE);
		//! Check if the instance was created
		bool FlagCreate(void) { return isREMOTEThreadRunning; }
		//! Destroy the instance
		virtual void Destroy(void);
		//! Returns the ID of the REMOTE::Thread.
		int Id();

		friend void runREMOTEThread(REMOTE::Thread *thread);

		virtual void t_Main(void) = 0;

		//! Configuration constants
		enum PTConfigType {
			CONFIG_STACK_SIZE = 4096
		};

	protected:

		//! The thread main virtual function
		//        virtual void t_Main(void) = 0;
		/*!<
		* This function is the thread main virtual function.
		* As soon as a thread starts, this function is called.
		* When this function returns, the thread is terminated.
		*
		* You have to define this function when you define a new
		* class that inherits REMOTE::Thread class.
		*/

	private:
		bool isREMOTEThreadRunning;
		boost::mutex REMOTEThreadMutex;
    };
}

#endif /* REMOTETHREAD_H_ */
