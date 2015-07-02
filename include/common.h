/*
 * tel-plugin-nitz
 *
 * Copyright (c) 2000 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <glib.h>
#include <gio/gio.h>

#include <unicode/ustring.h>
#include <unicode/ucal.h>

#ifdef FEATURE_FILE_LOG

/* filelog */
#ifndef NITZ_LOG_FILE
#define NITZ_LOG_FILE		"/opt/var/log/nitz.log"
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

#else
#define filelog(fmt, args...) warn(fmt, ##args);
#endif

#define NITZ_TIMEZONE_MAX_LEN 64

struct nitz_custom_data {
	gboolean nitz_updated;
	gboolean need_fix_zone;
	gboolean wait_for_icc_card;
	gboolean wait_for_net_registration;

	char *plmn;
	GHashTable *mcctable_hash;

	GQueue *nitz_pending_queue;
	CoreObject *co_network;
	TcorePlugin *plugin;
};

typedef enum ConnMode {
	CONN_MODE_COMPAINION,
	CONN_MODE_REMOTE,
	CONN_MODE_STANDALONE,
	CONN_MODE_UNKNOWN
} ConnMode;

char * __nitz_get_country_code_for_mcc(char *operator_mcc, struct nitz_custom_data *custom_data);

#endif
