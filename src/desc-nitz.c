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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <vconf.h>

#include <tcore.h>
#include <server.h>
#include <plugin.h>
#include <storage.h>
#include <co_network.h>

#include "common.h"
#include "time_update.h"
#ifdef TIZEN_FEATURE_TEST_ENABLE
#include "nitz_test.h"
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#define MCCTABLE_DIR_PATH "/opt/data/etc/mcctable.xml"

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION 1
#endif

#define VCONFKEY_WECONN_ALL_CONNECTED	"memory/private/weconn/all_connected"
#define VCONFKEY_SAP_CONNECTION_TYPE	"memory/private/sap/conn_type"
#define PROP_NET_NITZ_TIMEZONE			"nitz_timezone"

typedef enum SapConnType {
	SAP_ALL = 0x00, /* All connectivity */
	SAP_BT = 0x01, /* BT connectivity */
	SAP_BLE = 0x02, /* BLE connectivity */
	SAP_TCP_IP = 0x04, /* TCP connectivity */
	SAP_USB = 0x08, /* USB connectivity */
	SAP_MOBILE = 0x10, /* SCS connectivity */
	SAP_CONNECTIVITY_END = 0xFF /* End of Connectivity */
} SapConnType;

struct nitz_time_update_s {
	CoreObject *co_network;

	struct tnoti_network_timeinfo *timeinfo;
	struct nitz_custom_data *custom_data;
	struct timespec curtime;
};

static void __load_XML(char *docname, char *groupname, void **i_doc, void **i_root_node)
{
	xmlDocPtr *doc = (xmlDocPtr *)i_doc;
	xmlNodePtr *root_node = (xmlNodePtr *)i_root_node;

	dbg("docname:%s, groupname:%s", docname, groupname);

	*doc = xmlParseFile(docname);
	if (*doc) {
		*root_node = xmlDocGetRootElement(*doc);
		if (*root_node) {
			dbg("*root_node->name:%s", (*root_node)->name);
			if (0 == xmlStrcmp((*root_node)->name, (const xmlChar *)groupname)) {
				*root_node = (*root_node)->xmlChildrenNode;
			} else {
				xmlFreeDoc(*doc);
				*doc = NULL;
				*root_node = NULL;
			}
		} else {
			xmlFreeDoc(*doc);
			*doc = NULL;
		}
	} else
		err("Cannot parse doc(%s)", docname);
}

static void __unload_XML(void **i_doc, void **i_root_node)
{
	xmlDocPtr *doc = (xmlDocPtr *)i_doc;
	xmlNodePtr *root_node = (xmlNodePtr *)i_root_node;

	dbg("unloading XML");
	if (doc && *doc) {
		xmlFreeDoc(*doc);
		*doc = NULL;
		if (root_node)
			*root_node = NULL;
	}
}

char *__nitz_get_country_code_for_mcc(char *operator_mcc, struct nitz_custom_data *data)
{
	xmlNodePtr cur = NULL;
	xmlNode *cur_node = NULL;
	char *mcc_str = NULL, *iso = NULL;
	void *xml_doc = NULL, *xml_root_node = NULL;

	dbg("operator_mcc = %s", operator_mcc);

	iso = (char *)g_hash_table_lookup(data->mcctable_hash, operator_mcc);
	if (iso) {
		dbg("Found a record in cache (mcc[%s], iso[%s])", operator_mcc, iso ? iso : "");
		return g_strdup(iso);
	}

	__load_XML(MCCTABLE_DIR_PATH, "mcctable", &xml_doc, &xml_root_node);
	if (!xml_root_node) {
		err("mcctable.xml load error");
		return NULL;
	}

	/* Get iso country code from mcctable for mcc */
	cur = xml_root_node;
	for (cur_node = cur; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			mcc_str = (char *)xmlGetProp(cur_node, (const xmlChar *)"mcc");
			if (g_strcmp0(operator_mcc, mcc_str) == 0) {
				iso = g_strdup((char *)xmlGetProp(cur_node, (const xmlChar *)"iso"));
				dbg("Found a record(mcc[%s], iso[%s])", mcc_str, iso ? iso : "");
				break;
			}
		}
	}
	__unload_XML(&xml_doc, &xml_root_node);
	if (iso)
		g_hash_table_insert(data->mcctable_hash, g_strdup(operator_mcc), g_strdup(iso));
	return iso;
}

static enum ConnMode __nitz_get_device_connection_mode(void)
{
#ifndef TIZEN_FEATURE_COMPANION_MODE_ENABLE
	return CONN_MODE_STANDALONE;
#else
	int all_connected = FALSE;
	int sap_conn_type = 0;

	vconf_get_int(VCONFKEY_WECONN_ALL_CONNECTED, &all_connected);
	vconf_get_int(VCONFKEY_SAP_CONNECTION_TYPE, &sap_conn_type);
	info("(%s : %d) (%s : %d)", VCONFKEY_WECONN_ALL_CONNECTED, all_connected, VCONFKEY_SAP_CONNECTION_TYPE, sap_conn_type);

	if (!all_connected) {
		return CONN_MODE_STANDALONE;
	} else {
		switch (sap_conn_type) {
		case SAP_BT:
			return CONN_MODE_COMPAINION;

		case SAP_MOBILE:
			return CONN_MODE_REMOTE;

		default:
		err("unknown type");
		break;
		}
	}
	return CONN_MODE_UNKNOWN;
#endif
}

static gboolean __nitz_is_airplain_mode(void)
{
	gboolean flightmode = FALSE;
	vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &flightmode);
	return flightmode;
}

static gboolean _on_timeout_timezone_nitz_not_update(gpointer user_cb_data)
{
	struct nitz_custom_data *custom_data = user_cb_data;

	if(!custom_data)
		return G_SOURCE_REMOVE;

	if(CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
		info("device is in companion mode! do not ask to user");
	} else if (FALSE == nitz_is_auto_timezone()) {
		info("Already manual timezone! do not ask to user");
	} else if (TRUE == __nitz_is_airplain_mode()) {
		info("Flight Mode! do not ask to user");
	} else {
		info("nitz not updated. ask to user");
		tcore_object_set_property(custom_data->co_network,
			PROP_NET_NITZ_TIMEZONE, "user_selection_required");
	}

	custom_data->timezone_sel_timer_id = 0;
	return G_SOURCE_REMOVE;
}

static void _wait_nitz_and_timezone_select_by_user(struct nitz_custom_data *custom_data)
{
	if(!custom_data)
		return;

	custom_data->timezone_sel_timer_id = g_timeout_add_seconds(NITZ_TIMEZONE_SELECT_WAITING_TIMEOUT,
		_on_timeout_timezone_nitz_not_update, custom_data);
	dbg("Add timer(%d) timeout(%d)", custom_data->timezone_sel_timer_id, NITZ_TIMEZONE_SELECT_WAITING_TIMEOUT);
}

static void _stop_nitz_waiting(struct nitz_custom_data *custom_data)
{
	if (custom_data && custom_data->timezone_sel_timer_id != 0) {
		gboolean ret;

		ret = g_source_remove(custom_data->timezone_sel_timer_id);
		if (FALSE == ret)
			warn("g_source_remove fail");
		else
			dbg("Stopped");

		custom_data->timezone_sel_timer_id = 0;
	}
}

static void _check_fix_multi_time_zone(struct nitz_custom_data *custom_data,
	int mcc, char *iso, char *timezone_name)
{
	if (!custom_data || !iso)
		return;

	info("We have no way to select timezone. waiting NITZ and ask to user.");
	_wait_nitz_and_timezone_select_by_user(custom_data);
}

static void _check_fix_time_zone(struct nitz_custom_data *custom_data)
{
	char mcc_str[4] = {0,};
	char *iso = NULL;
	int mcc = 0;
	int icc_card_exist = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	enum telephony_network_service_type svc_type = NETWORK_SERVICE_TYPE_UNKNOWN;
	Server *s = tcore_plugin_ref_server(custom_data->plugin);
	GList *tz_list = NULL;
	char *timezone_name = NULL;
	int tz_count = 0;

	if (!custom_data->need_fix_zone) {
		info("We don't need to fix timezone");
		return;
	}

	if (!nitz_is_auto_timezone()) {
		info("We don't need to fix timezone in Manual timezone mode");
		return;
	}

	vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &icc_card_exist);
	if (icc_card_exist != VCONFKEY_TELEPHONY_SIM_INSERTED) {
		if (tcore_server_get_modems_count(s) == 2) {
			int icc_card2_exist = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
			vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT2, &icc_card2_exist);
			if (icc_card2_exist != VCONFKEY_TELEPHONY_SIM_INSERTED) {
				info("We don't need to fix timezone. Icc card is not exist.");
				custom_data->wait_for_icc_card = TRUE;
				return;
			}
		} else {
			info("We don't need to fix timezone. Icc card is not exist.");
			custom_data->wait_for_icc_card = TRUE;
			return;
		}
	}

	tcore_network_get_service_type(custom_data->co_network, &svc_type);
	if (svc_type <= NETWORK_SERVICE_TYPE_SEARCH) {
		info("We don't need to fix timezone. Not registered network yet");
		custom_data->wait_for_net_registration = TRUE;
		return;
	} else {
		custom_data->wait_for_net_registration = FALSE;
	}

	memcpy(mcc_str, custom_data->plmn, 3);
	mcc = atoi(mcc_str);
	iso = __nitz_get_country_code_for_mcc(mcc_str, custom_data);
	if (iso) {
		tz_list = nitz_get_tzlist(iso);
		tz_count =  g_list_length(tz_list);
		dbg("tz_count - %d", tz_count);
		if (tz_count > 0) {
			if (nitz_is_iso_changed(iso)) {
				timezone_name = tz_list->data;
				if (tz_count > 1) {
					info(" Multi timezone");
					_check_fix_multi_time_zone(custom_data, mcc, iso, timezone_name);
				} else {
					info("Single timezone. Apply (mcc:[%d] iso:[%s] zone:[%s])", mcc, iso, timezone_name);
					nitz_apply_tzfile (timezone_name);
					_stop_nitz_waiting(custom_data);
				}
				nitz_set_iso(iso);
			} else {
				info("Same Area. Do not change timezone now");
			}
			goto EXIT;
		} else {
			info("timezone not found for mcc[%d] and iso[%s]", mcc, iso);
		}
	} else {
		dbg("country code is NULL for mcc %d", mcc);
	}

	switch (mcc) {
		case 1:
		case 999:
			info("Exceptional mcc(%d). do not ask timezone to user", mcc);
			mcc = -1;
		break;
		default:
		break;
	}
	if (mcc >= 0) {
		info("Cannot find. Timezone for mcc:[%d]. Ask to user", mcc);
		_wait_nitz_and_timezone_select_by_user(custom_data);
	}
EXIT:
	g_list_free_full(tz_list, g_free);
	g_free(iso);
}

static gboolean _update_network_timeinfo(gpointer user_data)
{
	struct nitz_time_update_s *time_update = user_data;
	struct tnoti_network_timeinfo *timeinfo = NULL;
	struct nitz_custom_data *custom_data = NULL;

	if  (!time_update)
		return FALSE;

	timeinfo = time_update->timeinfo;
	custom_data = time_update->custom_data;
	custom_data->nitz_updated = TRUE;
	custom_data->need_fix_zone = FALSE;

	info("[TIMESTAMP][STEP3-Processing] %04d-%02d-%02d %02d:%02d:%02d wday=%d GMT:%d, dstoff:%d, is_dst:%d",
			timeinfo->year, timeinfo->month, timeinfo->day,
			timeinfo->hour, timeinfo->minute, timeinfo->second,
			timeinfo->wday, timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

	if (CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
		g_free(timeinfo);
		g_free(time_update);
		info("[TIMESTAMP] device is in companion mode! ignore NITZ update");
		return FALSE;
	}

	if (!custom_data->co_network)
		custom_data->co_network = time_update->co_network;

	if (strlen(timeinfo->plmn) == 0 && custom_data->plmn != NULL) {
		/* Updated PLMN from custom_data incase when received PLMN is empty. */
		info("Received PLMN is empty, so use cached PLMN");
		g_strlcpy(timeinfo->plmn, custom_data->plmn, strlen(custom_data->plmn)+1);
	}

	if (nitz_time_update(timeinfo, custom_data, time_update->curtime)) {
		_stop_nitz_waiting(custom_data);
	}

	g_free(timeinfo);
	g_free(time_update);

	return FALSE;
}

static gboolean _process_nitz_pending(gpointer user_data)
{
	struct nitz_custom_data *custom_data = user_data;
	struct nitz_time_update_s *time_update = NULL;
	struct tnoti_network_timeinfo *timeinfo = NULL;

	if (!custom_data)
		return FALSE;

	while (1) {
		time_update = g_queue_pop_head(custom_data->nitz_pending_queue);
		if (!time_update)
			break;

		timeinfo = time_update->timeinfo;
		if (!timeinfo)
			continue;

		if (g_queue_is_empty(custom_data->nitz_pending_queue))
			break;

		info("[TIMESTAMP][STEP2-Discard] %04d-%02d-%02d %02d:%02d:%02d wday=%d GMT:%d, dstoff:%d, is_dst:%d",
				timeinfo->year, timeinfo->month, timeinfo->day,
				timeinfo->hour, timeinfo->minute, timeinfo->second,
				timeinfo->wday, timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

		g_free(timeinfo);
		g_free(time_update);
	}

	_update_network_timeinfo(time_update);
	return FALSE;
}

static enum tcore_hook_return on_hook_network_regist(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	struct nitz_custom_data *custom_data = user_data;
	struct tnoti_network_registration_status *info = data;

	if (!custom_data->co_network)
		custom_data->co_network = source;

	/*
	 * Handle MCC changed scenario -
	 * If MCC is changed and network is registered, update timezone
	 */
	if ((info->service_type >= NETWORK_SERVICE_TYPE_SEARCH)
		&& (custom_data->wait_for_net_registration))
			_check_fix_time_zone(custom_data);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network(Server *s, CoreObject *source,
	enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	struct nitz_custom_data *custom_data = user_data;
	struct tnoti_network_change *info = data;
	gboolean is_mcc_changed = FALSE;

	if (!info || strlen(info->plmn) == 0) {
		info("Invalid data");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	if (!custom_data->co_network)
		custom_data->co_network = source;

	/*
	 * Handle MCC changed scenario -
	 * If MCC is changed and network is registered, update timezone
	 */
	if ((custom_data->plmn && strncmp(info->plmn, custom_data->plmn, 3)) || (!custom_data->plmn)) {
		info("MCC change (%s) -> (%s)", custom_data->plmn ? custom_data->plmn : "", info->plmn);
		g_free(custom_data->plmn);
		custom_data->plmn = g_strdup(info->plmn);
		is_mcc_changed = TRUE;
	}

	if (is_mcc_changed) {
		if (CONN_MODE_COMPAINION == __nitz_get_device_connection_mode()) {
			info("device is in companion mode! ignore MCC change");
		} else {
			custom_data->need_fix_zone = TRUE;
			_check_fix_time_zone(custom_data);
		}
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_timeinfo(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	struct nitz_time_update_s *time_update;
	struct tnoti_network_timeinfo *timeinfo = data;
	struct nitz_custom_data *custom_data = user_data;
	struct timespec curtime;

	clock_gettime(CLOCK_REALTIME, &curtime);

	filelog("NITZ !! (time(NULL) = %u)", (unsigned int)time(NULL));

	info("[TIMESTAMP][STEP1-AddQueue] %04d-%02d-%02d %02d:%02d:%02d wday=%d GMT:%d, dstoff:%d, is_dst:%d",
			timeinfo->year, timeinfo->month, timeinfo->day,
			timeinfo->hour, timeinfo->minute, timeinfo->second,
			timeinfo->wday, timeinfo->gmtoff, timeinfo->dstoff, timeinfo->isdst);

	/* Allocate memory */
	time_update = g_try_malloc0(sizeof(struct nitz_time_update_s));
	if (time_update == NULL) {
		err("Memory allocation failed");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	time_update->custom_data = custom_data;
	time_update->timeinfo = g_memdup(timeinfo, sizeof(struct tnoti_network_timeinfo));
	time_update->co_network = source;
	time_update->curtime.tv_sec = curtime.tv_sec;
	time_update->curtime.tv_nsec = curtime.tv_nsec;

	if (g_queue_is_empty(custom_data->nitz_pending_queue)) {
		info("No pending NITZ");
		g_idle_add(_process_nitz_pending, custom_data);
	}
	info("Add nitz info to queue");
	g_queue_push_tail(custom_data->nitz_pending_queue, time_update);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static void on_vconf_sim_slot_state(keynode_t *node, void *user_data)
{
	int icc_card_exist = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	struct nitz_custom_data *data = user_data;
	Server *s = tcore_plugin_ref_server(data->plugin);

	vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &icc_card_exist);
	if (icc_card_exist == VCONFKEY_TELEPHONY_SIM_INSERTED)
		goto UPDATE;
	else {
		if (tcore_server_get_modems_count(s) == 2) {
			int icc_card2_exist = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
			vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT2, &icc_card2_exist);
			if (icc_card2_exist == VCONFKEY_TELEPHONY_SIM_INSERTED)
				goto UPDATE;
		}
		return;
	}
UPDATE:
	{
		struct nitz_custom_data *data = user_data;
		if (data->wait_for_icc_card) {
			info("ICC CARD INSERTED. check timezone again");
			_check_fix_time_zone(user_data);
			data->wait_for_icc_card = FALSE;
		}
	}
}

static gboolean on_load()
{
	dbg("i'm load!");

	return TRUE;
}

static gboolean on_init(TcorePlugin *p)
{
	Server *s;
	struct nitz_custom_data *data;
	guint modems_count;

	if (!p)
		return FALSE;

	dbg("i'm init!");

	s = tcore_plugin_ref_server(p);
	if (!s)
		return FALSE;

	data = calloc(sizeof(struct nitz_custom_data), 1);
	if (!data)
		return FALSE;

	data->plugin = p;
	tcore_plugin_link_user_data(p, data);

	data->nitz_updated = FALSE;
	data->need_fix_zone = FALSE;
	data->mcctable_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	data->nitz_pending_queue = g_queue_new();
	g_queue_init(data->nitz_pending_queue);

	tcore_server_add_notification_hook(s, TNOTI_NETWORK_CHANGE, on_hook_network, data);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_REGISTRATION_STATUS, on_hook_network_regist, data);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_TIMEINFO, on_hook_network_timeinfo, data);

	vconf_notify_key_changed(VCONFKEY_TELEPHONY_SIM_SLOT, on_vconf_sim_slot_state, data);

	modems_count = tcore_server_get_modems_count(s);
	dbg("get modems count - %d", modems_count);
	if (modems_count == 2)
		vconf_notify_key_changed(VCONFKEY_TELEPHONY_SIM_SLOT2, on_vconf_sim_slot_state, data);

#ifdef TIZEN_FEATURE_TEST_ENABLE
	nitz_test_init(p);
#endif

	return TRUE;
}

static void on_unload(TcorePlugin *p)
{
	struct nitz_custom_data *data;
	Server *s;
	guint modems_count;

	dbg("i'm unload");

#ifdef TIZEN_FEATURE_TEST_ENABLE
	nitz_test_deinit(p);
#endif

	data = tcore_plugin_ref_user_data(p);
	if (!data)
		return;

	if (data->plmn)
		g_free(data->plmn);

	if (data->mcctable_hash)
		g_hash_table_destroy(data->mcctable_hash);

	s = tcore_plugin_ref_server(p);
	tcore_server_remove_notification_hook(s, on_hook_network);
	tcore_server_remove_notification_hook(s, on_hook_network_regist);
	tcore_server_remove_notification_hook(s, on_hook_network_timeinfo);

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SIM_SLOT, on_vconf_sim_slot_state);
	modems_count = tcore_server_get_modems_count(s);
	dbg("get modems count - %d", modems_count);
	if (modems_count == 2)
		vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SIM_SLOT2, on_vconf_sim_slot_state);

	if (data->timezone_sel_timer_id != 0)
		g_source_remove(data->timezone_sel_timer_id);
	free(data);
}

EXPORT_API struct tcore_plugin_define_desc plugin_define_desc = {
	.name = "NITZ",
	.priority = TCORE_PLUGIN_PRIORITY_MID,
	.version = PLUGIN_VERSION,
	.load = on_load,
	.init = on_init,
	.unload = on_unload
};
