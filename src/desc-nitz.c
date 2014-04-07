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

static TcoreHookReturn on_hook_network_timeinfo(TcorePlugin *source,
			TcoreNotification command,
			guint data_len, void *data, void *user_data)
{
	TelNetworkNitzInfoNoti *timeinfo = data;
	gboolean flag_auto_update = FALSE;

	filelog("NITZ !! (time(NULL) = %u)", (guint)time(NULL));
	dbg("+- %04d-%02d-%02d %02d:%02d:%02d",
			timeinfo->year, timeinfo->month, timeinfo->day,
			timeinfo->hour, timeinfo->minute, timeinfo->second);
	dbg("+- GMT-offset:%d, DST-offset:%d, is_dst:%d",
			timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL,
							&flag_auto_update);
	if (flag_auto_update == FALSE)
		dbg("manual time update mode");

	nitz_time_update(timeinfo, flag_auto_update);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_modem_plugin_added(Server *s,
			TcoreServerNotification command,
			guint data_len, void *data, void *user_data)
{
	TcorePlugin *modem_plugin;

	modem_plugin = (TcorePlugin *)data;
	tcore_check_return_value_assert(NULL != modem_plugin, TCORE_HOOK_RETURN_STOP_PROPAGATION);

	tcore_plugin_add_notification_hook(modem_plugin, TCORE_NOTIFICATION_NETWORK_TIMEINFO,
					on_hook_network_timeinfo, NULL);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_modem_plugin_removed(Server *s,
			TcoreServerNotification command,
			guint data_len, void *data, void *user_data)
{
	TcorePlugin *modem_plugin;

	modem_plugin = (TcorePlugin *)data;
	tcore_check_return_value_assert(NULL != modem_plugin, TCORE_HOOK_RETURN_STOP_PROPAGATION);

	tcore_plugin_remove_notification_hook(modem_plugin, TCORE_NOTIFICATION_NETWORK_TIMEINFO,
					on_hook_network_timeinfo);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static gboolean on_load()
{
	dbg("Load!!!");
	return TRUE;
}

static gboolean on_init(TcorePlugin *plugin)
{
	Server *s;
	GSList *list = NULL;
	TcorePlugin *modem_plugin;

	tcore_check_return_value_assert(NULL != plugin, FALSE);

	dbg("Init!!!");

	s = tcore_plugin_ref_server(plugin);
	list = tcore_server_get_modem_plugin_list(s);
	while (list) {	/* Process for pre-loaded Modem Plug-in */
		modem_plugin = list->data;
		if ( NULL != modem_plugin)
			tcore_plugin_add_notification_hook(modem_plugin, TCORE_NOTIFICATION_NETWORK_TIMEINFO,
					on_hook_network_timeinfo, NULL);
		list = g_slist_next(list);
	}
	g_slist_free(list);

	/* Register for post-loaded Modem Plug-ins */
	tcore_server_add_notification_hook(s, TCORE_SERVER_NOTIFICATION_ADDED_MODEM_PLUGIN,
					on_hook_modem_plugin_added, NULL);
	tcore_server_add_notification_hook(s, TCORE_SERVER_NOTIFICATION_REMOVED_MODEM_PLUGIN,
					on_hook_modem_plugin_removed, NULL);
	return TRUE;
}

static void on_unload(TcorePlugin *plugin)
{
	Server *s;

	tcore_check_return_assert(NULL != plugin);
	dbg("Unload");

	s = tcore_plugin_ref_server(plugin);
	tcore_server_remove_notification_hook(s, on_hook_modem_plugin_added);
	tcore_server_remove_notification_hook(s, on_hook_modem_plugin_removed);
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
