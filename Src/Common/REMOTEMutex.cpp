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
 * REMOTEMutex.h
 *
 *  Created on: 31-08-2013
 *      Author: shivaram.y
 */

#include "REMOTEMutex.h"

/*!
 * This function creates a PCMutex object.
 *
 * \return true on success, otherwise false.
 * \see Destroy()
 */
bool REMOTE::Mutex::Create(void)
{
	ASSERT(FlagCreate() == false);

	if (m_sem.Create(1) == false)
		return false;

	m_threadId = 0;

	return true;
}

/*!
 * This function destroys the PCMutex object.
 *
 * \return true on success, otherwise false.
 */
void REMOTE::Mutex::Destroy(void)
{
	ASSERT(FlagCreate() == true);

	m_sem.Destroy();

	return;
}

/*!
 * This function locks the created PCMutex object.
 *
 * \see Unlock(), Try()
 */
void REMOTE::Mutex::Lock(void)
{
	ASSERT(FlagCreate() == true);

	if (m_threadId == (int)pthread_self()) {
		m_count++;

		return;
	}

	m_sem.Take();

	m_threadId = (int)pthread_self();
	m_count = 1;

	return;
}

/*!
 * If this function succeeds, the memeber variable that saves the
 * thread ID will be set to the current thread ID.
 *
 * \return true on success, otherwise false.
 * \see Lock(), Unlock()
 */
bool REMOTE::Mutex::Try(unsigned long msec)
{
	ASSERT(FlagCreate() == true);

	if (m_threadId == (int)pthread_self()) {
		m_count++;

		return true;
	}

	if (m_sem.Try(msec) == false)
		return false;

	m_threadId = (int)pthread_self();
	m_count = 1;

	return true;
}

/*
 * This function unlocks the PCMutex object.
 *
 * \return true on success, otherwise false.
 * \see Lock(), Try()
 */
bool REMOTE::Mutex::Unlock(void)
{
	ASSERT(FlagCreate() == true);

	if (m_threadId != (int)pthread_self())
		return false;

	m_count--;

	if (m_count > 0)
		return true;

	m_threadId = 0;

	m_sem.Give();

	return true;
}
