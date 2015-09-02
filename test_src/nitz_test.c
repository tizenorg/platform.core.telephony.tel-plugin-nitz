/*
 * tel-plugin-nitz
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Ja-young Gu <jygu@samsung.com>
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

#include "nitz_test.h"

#include "common.h"
#include "time_update.h"

#include <glib.h>
#include <string.h>
#include <vconf.h>
#include <plugin.h>

#define NITZ_VCONF_LAUNCH_TEST0	"memory/private/telephony/nitz_test0"
#define NITZ_VCONF_LAUNCH_TEST1	"memory/private/telephony/nitz_test1"
#define NITZ_VCONF_LAUNCH_TEST2	"memory/private/telephony/nitz_test2"

/* FIXME: Make better APIs in plugin */
extern enum ConnMode __nitz_get_device_connection_mode(void);
extern void _check_fix_time_zone(struct nitz_custom_data *custom_data);

static void on_hook_test0(keynode_t *node, void *user_data);
static void on_hook_test1(keynode_t *node, void *user_data);
static void on_hook_test2(keynode_t *node, void *user_data);

void on_hook_test0(keynode_t *node, void *user_data)
{
	int rv = 0;

	if (vconf_get_int(NITZ_VCONF_LAUNCH_TEST0, &rv) == 0
		&& rv == 1) {

		struct nitz_custom_data *custom_data = tcore_plugin_ref_user_data((TcorePlugin *)user_data);
		struct tnoti_network_timeinfo *timeinfo;
		char *test_plmn = "31000";
		struct timespec curtime= {0,};

		clock_gettime(CLOCK_REALTIME, &curtime);
		dbg("[RUNNING TEST] on_hook_test0()");

		timeinfo = g_malloc0(sizeof(struct tnoti_network_timeinfo));

		timeinfo->year = 14;
		timeinfo->month = 11;
		timeinfo->day = 2;
		timeinfo->hour = 5;
		timeinfo->minute = 59;
		timeinfo->second = 19;
		timeinfo->wday = 6;
		timeinfo->gmtoff = -240;
		timeinfo->dstoff = 1;
		timeinfo->isdst = 1;
		g_strlcpy(timeinfo->plmn, test_plmn, 7);

		filelog("NITZ !! (time(NULL) = %u)", (unsigned int)time(NULL));

		dbg(" +- %04d-%02d-%02d %02d:%02d:%02d wday=%d",
				timeinfo->year, timeinfo->month, timeinfo->day,
				timeinfo->hour, timeinfo->minute, timeinfo->second,
				timeinfo->wday);
		dbg(" +- GMT-offset:%d, DST-offset:%d, is_dst:%d",
				timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

		custom_data->nitz_updated = TRUE;
		custom_data->need_fix_zone = FALSE;

		if(CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
			dbg("device is in companion mode! ignore NITZ update");
			g_free(timeinfo);
			return;
		}

		nitz_time_update(timeinfo, custom_data, curtime);
		g_free(timeinfo);
	} /* End Test */
}

void on_hook_test1(keynode_t *node, void *user_data)
{
	struct nitz_custom_data *custom_data = tcore_plugin_ref_user_data((TcorePlugin *)user_data);
	struct tnoti_network_change *info = NULL;
	gboolean is_mcc_changed = FALSE;
	char *test_plmn = "310470";

	test_plmn = vconf_get_str(NITZ_VCONF_LAUNCH_TEST1);
	if (test_plmn == NULL) {
		err("PLMN is NULL");
		return;
	}

	if (strlen(test_plmn) == 0) {
		dbg("Invalid data");
		g_free(test_plmn);
		return;
	}

	dbg("[RUNNING TEST] on_hook_test1()");

	info = g_malloc0(sizeof(struct tnoti_network_change));
	g_strlcpy(info->plmn, test_plmn, 7);
	dbg("testplmn : %s", test_plmn);

	if ((custom_data->plmn && strncmp(info->plmn, custom_data->plmn, 3))
		|| (!custom_data->plmn)) {
		dbg("MCC change (%s) -> (%s)", custom_data->plmn?custom_data->plmn:"", info->plmn);
		g_free(custom_data->plmn);
		custom_data->plmn = g_strdup(info->plmn);
		is_mcc_changed = TRUE;
	}

	if (is_mcc_changed) {
		if(CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
			dbg("device is in companion mode! ignore MCC change");
		} else {
			custom_data->need_fix_zone = TRUE;
			_check_fix_time_zone(custom_data);
		}
	}

	g_free(info);
	g_free(test_plmn);
}

void on_hook_test2(keynode_t *node, void *user_data)
{

	int rv = 0;

	if (vconf_get_int(NITZ_VCONF_LAUNCH_TEST2, &rv) == 0
		&& rv == 1) {

		struct nitz_custom_data *custom_data = tcore_plugin_ref_user_data((TcorePlugin *)user_data);
		struct tnoti_network_timeinfo *timeinfo;
		char *test_plmn = "31000";
		struct timespec curtime= {0,};

		clock_gettime(CLOCK_REALTIME, &curtime);
		dbg("[RUNNING TEST] on_hook_test2()");

		timeinfo = g_malloc0(sizeof(struct tnoti_network_timeinfo));

		timeinfo->year = 14;
		timeinfo->month = 11;
		timeinfo->day = 2;
		timeinfo->hour = 6;
		timeinfo->minute = 0;
		timeinfo->second = 9;
		timeinfo->wday = 6;
		timeinfo->gmtoff = -300;
		timeinfo->dstoff = 0;
		timeinfo->isdst = 1;
		g_strlcpy(timeinfo->plmn, test_plmn, 7);

		filelog("NITZ !! (time(NULL) = %u)", (unsigned int)time(NULL));

		dbg(" +- %04d-%02d-%02d %02d:%02d:%02d wday=%d",
				timeinfo->year, timeinfo->month, timeinfo->day,
				timeinfo->hour, timeinfo->minute, timeinfo->second,
				timeinfo->wday);
		dbg(" +- GMT-offset:%d, DST-offset:%d, is_dst:%d",
				timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

		custom_data->nitz_updated = TRUE;
		custom_data->need_fix_zone = FALSE;

		if(CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
			dbg("device is in companion mode! ignore NITZ update");
			g_free(timeinfo);
			return;
		}

		nitz_time_update(timeinfo, custom_data, curtime);
		g_free(timeinfo);
	} /* End Test */
}

void nitz_test_init(TcorePlugin *plugin)
{
	vconf_notify_key_changed(NITZ_VCONF_LAUNCH_TEST0, on_hook_test0, plugin);
	vconf_notify_key_changed(NITZ_VCONF_LAUNCH_TEST1, on_hook_test1, plugin);
	vconf_notify_key_changed(NITZ_VCONF_LAUNCH_TEST2, on_hook_test2, plugin);
}

void nitz_test_deinit(TcorePlugin *plugin)
{
	vconf_ignore_key_changed(NITZ_VCONF_LAUNCH_TEST0, on_hook_test0);
	vconf_ignore_key_changed(NITZ_VCONF_LAUNCH_TEST1, on_hook_test1);
	vconf_ignore_key_changed(NITZ_VCONF_LAUNCH_TEST2, on_hook_test2);
}
