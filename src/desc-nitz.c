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

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <vconf.h>

#include <tcore.h>
#include <server.h>
#include <plugin.h>
#include <storage.h>
#include <co_network.h>

#include "common.h"
#include "time_update.h"

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION 1
#endif

static enum tcore_hook_return on_hook_network_timeinfo(Server *s,
			CoreObject *source,
			enum tcore_notification_command command,
			unsigned int data_len, void *data, void *user_data)
{
	struct tnoti_network_timeinfo *timeinfo = data;
	gboolean flag_auto_update = FALSE;

	filelog("NITZ !! (time(NULL) = %u)", (unsigned int)time(NULL));
	dbg("+- %04d-%02d-%02d %02d:%02d:%02d wday=%d",
			timeinfo->year, timeinfo->month, timeinfo->day,
			timeinfo->hour, timeinfo->minute, timeinfo->second,
			timeinfo->wday);
	dbg("+- GMT-offset:%d, DST-offset:%d, is_dst:%d",
			timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL,
							&flag_auto_update);
	if (flag_auto_update == FALSE)
		dbg("manual time update mode");

	nitz_time_update(timeinfo, flag_auto_update);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static gboolean on_load()
{
	dbg("Load!!!");

	return TRUE;
}

static gboolean on_init(TcorePlugin *p)
{
	Server *s;

	dbg("Init!!!");

	s = tcore_plugin_ref_server(p);
	if (s == NULL)
		return FALSE;

	tcore_server_add_notification_hook(s, TNOTI_NETWORK_TIMEINFO,
					on_hook_network_timeinfo, NULL);

	return TRUE;
}

static void on_unload(TcorePlugin *p)
{
	dbg("Unload");
}

EXPORT_API struct tcore_plugin_define_desc plugin_define_desc =
{
	.name = "NITZ",
	.priority = TCORE_PLUGIN_PRIORITY_MID,
	.version = PLUGIN_VERSION,
	.load = on_load,
	.init = on_init,
	.unload = on_unload
};
