/*
 * tel-plugin-nitz
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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
 */

#ifndef __NITZ_COMMON_H__
#define __NITZ_COMMON_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* filelog */
#ifndef NITZ_LOG_FILE
#define NITZ_LOG_FILE	"/opt/var/log/nitz.log"
#endif

#ifndef NITZ_LOG_FUNC
#define NITZ_LOG_FUNC fprintf
#endif

#define filelog(fmt, args...) { \
		FILE *fp = fopen(NITZ_LOG_FILE, "a"); \
		dbg(fmt, ##args); \
		if (fp) { \
			NITZ_LOG_FUNC(fp, fmt "\n", ##args); \
			fflush(fp); \
			fclose(fp); \
		} \
	}

#endif
