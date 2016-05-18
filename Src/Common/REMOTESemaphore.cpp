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
 * REMOTESemaphore.h
 *
 *  Created on: 31-08-2013
 *      Author: shivaram.y
 */

#include "REMOTESemaphore.h"
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>
//#include <sys/resource.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define CONFIG_MAXIMUM_YIELD_COUNTER 16
#define INFINITY ~0
static unsigned char semCounter = CONFIG_MAXIMUM_YIELD_COUNTER;

struct PTSemaphore {
    pthread_cond_t  cond;
    int             count;
};

pthread_mutex_t TimeMutex;


/*!
 * This function creates a PCSemaphore object.
 *
 * \param count Semaphore counter
 * \return true on success, otherwise false.
 *
 * \see Destroy()
 */
bool REMOTE::Semaphore::Create(int count)
{
	pthread_mutex_lock(&TimeMutex);

	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	m_id = new PTSemaphore;

	if (m_id != NULL) {
		m_id->cond  = cond;
		m_id->count = count;
	}

	pthread_mutex_unlock(&TimeMutex);

	return m_id != NULL;
}

/*!
 * This function destroys the PCSemaphore object.
 *
 * \see Create()
 */
void REMOTE::Semaphore::Destroy()
{
	pthread_mutex_lock(&TimeMutex);

	pthread_cond_destroy(&m_id->cond);
	delete m_id;

	m_id = NULL;
	pthread_mutex_unlock(&TimeMutex);

	return;
}

void REMOTE::Semaphore::Take(void)
{
	pthread_mutex_lock(&TimeMutex);

	while (m_id->count <= 0) {
		pthread_cond_wait(&m_id->cond, &TimeMutex);
	}

	m_id->count--;

	pthread_mutex_unlock(&TimeMutex);

	return;
}

bool REMOTE::Semaphore::Try(unsigned long msec)
{
	if (msec == (unsigned long)INFINITY) {
		Take();

		return true;
	}

	pthread_mutex_lock(&TimeMutex);

	struct timeval  now;
	struct timespec timeout;
	int             ret = 0;
	bool            tf;

	if (msec == 0) {
		if (m_id->count <= 0) {
			tf = false;
		} else {
			tf = true;

			m_id->count--;
		}
	} else {
		while ((m_id->count <= 0) && (ret != ETIMEDOUT)) {
			gettimeofday(&now, NULL);
			timeout.tv_sec  = now.tv_sec + msec / 1000;
			timeout.tv_nsec = now.tv_usec + msec % 1000 * 1000;

			while (timeout.tv_nsec > 1000000) {
				timeout.tv_nsec -= 1000000;
				timeout.tv_sec++;
			}

			timeout.tv_nsec *= 1000;

			ret = pthread_cond_timedwait(&m_id->cond, &TimeMutex, &timeout);
		}

		if (ret == ETIMEDOUT) {
			tf = false;
		} else {
			tf = true;

			m_id->count--;
		}
	}

	pthread_mutex_unlock(&TimeMutex);

	return tf;
}

void REMOTE::Semaphore::Give(void)
{
	pthread_mutex_lock(&TimeMutex);
	m_id->count++;
	pthread_cond_signal(&m_id->cond);
	pthread_mutex_unlock(&TimeMutex);

	if (!semCounter--) {
		sched_yield();
		semCounter = CONFIG_MAXIMUM_YIELD_COUNTER;
	}

	return;
}

