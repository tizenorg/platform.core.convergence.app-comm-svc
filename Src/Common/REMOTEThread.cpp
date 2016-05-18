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

/*
 * REMOTETime.h
 *
 *  Created on: 31-08-2013
 *      Author: shivaram.y
 */

/**
 * @file    REMOTEThread.cpp
 * @author  Shivaram Yadav (shivaram.y@samsung.com)
 * @version 1.0
 * @brief   REMOTEThread source file
 */

#include "REMOTEThread.h"

REMOTE::Thread::Thread()
{
	isREMOTEThreadRunning = false;
}

REMOTE::Thread::~Thread()
{
	this->Destroy();
}

/*!
 * This function starts REMOTEThread execution.
 *
 * \param thread REMOTEThread instance
 * \see Create()
 *
 */
void runREMOTEThread(REMOTE::Thread *thread)
{
	thread->t_Main();
}

/*!
 * This function creates REMOTEThread object.
 *
 * \param stackSize Size of stack
 *
 * \return true on success, otherwise false.
 *
 * \see Destroy()
 */
bool REMOTE::Thread::Create(unsigned long stackSize)
{
	boost::mutex::scoped_lock REMOTEScopedLock(REMOTEThreadMutex);

	ASSERT(FlagCreate() == false);

//    It seems that boost:thread::attributes are not available in boost on Tizen,

//    Setting REMOTE Thread attributes:
//    boost::thread::attributes REMOTEThreadAttributes;
//    REMOTEThreadAttributes.set_stack_size(stackSize);

//    Starting REMOTE Thread:
//    boost::thread REMOTEThread(REMOTEThreadAttributes, runREMOTEThread, this);

	boost::thread REMOTEThread(runREMOTEThread, this);
	isREMOTEThreadRunning = true;

	return true;
}

/*!
 * This function destroys the REMOTEThread.
 *
 * \see Create()
 */
void REMOTE::Thread::Destroy(void)
{
	isREMOTEThreadRunning = false;
	return;
}

/*!
 * This function returns REMOTEThread Id.
 *
 * \return threadNumber
 */
int REMOTE::Thread::Id(void)
{
	std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
	unsigned long threadNumber = 0;
	sscanf(threadId.c_str(), "%lx", &threadNumber);
	return threadNumber;
}

