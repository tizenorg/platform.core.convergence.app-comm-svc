
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
 * @file        REMOTEString.h   
 * @brief       String class provides functions that manage the String. 
 * @created on: 2013.08.31
 * @author: Shivaram Yadav (shivaram.y@samsung.com)
 * @reviewer: Ji-Hoon Kim(jihoon95.kim@samsung.com), Han Kang (han264,kang@samsung.com)
 */

#ifndef REMOTESTRING_H_
#define REMOTESTRING_H_

#include <string.h>
#include <string>
#include <vector>

#include "REMOTECommon.h"

using namespace std;

namespace REMOTE {
	class String {
	private:
		String(void) {}

	public:
		//! Compares strings.
		static int          Diff(const char *dest, const char *source);
		//! Compares strings.
		static int          Diff(const char *dest, const char *source, unsigned long nSize);
		//! Copies characters of one string to another.
		static void         Copy(char *dest, const char *source);
		//! Copies characters of one string to another.
		static void         Copy(char *dest, const char *source, unsigned long nSize);
		//! Gets the length of a string.
		static unsigned int Length(const char *pString);

		static std::string ReplaceAll(const std::string& targetString, const std::string& src, const std::string& dest);

		// split targetString to tokens datastructure.
		static int Split(const std::string& targetString, std::vector<std::string>& tokens, const std::string& delimiters);
	};
}

#endif /* REMOTESTRING_H_ */
