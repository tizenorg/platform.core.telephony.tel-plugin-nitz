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
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/sysinfo.h>

#include <glib.h>
#include <vconf.h>
#include <tcore.h>
#include <alarm.h>
#include <device/power.h>

#include "common.h"
#include "time_update.h"

#define BUF_SIZE 255
#define PATH_LOCALTIME	"/opt/etc/localtime"
#define PATH_ZONEINFO	"/usr/share/zoneinfo"
#define VCONFKEY_TELEPHONY_NITZ_ISO "db/private/telephony/nitz_iso"

gboolean nitz_is_auto_timezone(void)
{
	gboolean auto_timezone;
	vconf_get_bool(VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL, &auto_timezone);
	if (auto_timezone == FALSE) {
		dbg("Manual time update mode.");
		return FALSE;
	} else {
		dbg("Automatic time update mode.");
		return TRUE;
	}
}

gboolean nitz_is_iso_changed(char *iso)
{
	char *saved_iso = NULL;
	gboolean area_changed = TRUE;

	saved_iso = vconf_get_str (VCONFKEY_TELEPHONY_NITZ_ISO);
	if (saved_iso) {
		info("saved iso:[%s], mcc iso:[%s]", saved_iso, iso);
		if (g_ascii_strcasecmp(saved_iso, iso) == 0) {
			area_changed = FALSE;
		}
		free(saved_iso);
	}
	return area_changed;
}

void nitz_set_iso(char *iso)
{
	int ret = 0;

	if (!iso)
		return;

	ret = vconf_set_str (VCONFKEY_TELEPHONY_NITZ_ISO, iso);
	if (ret < 0) {
		warn("Fail to save iso:[%s]. ret=%d", iso, ret);
	} else {
		info("iso saved:[%s]", iso);
	}
}

static int nitz_get_uptime(void)
{
	struct sysinfo sys_info = {0,};

	if (sysinfo(&sys_info) == 0) {
		info("uptime: %ld secs", sys_info.uptime);
		return sys_info.uptime;
	}

	return 0;
}

gboolean nitz_set_time(struct timespec newtime, struct timespec curtime)
{
	int ret = 0;

	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_GMT, newtime.tv_sec);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_EVENT_GMT, nitz_get_uptime());

	if (!nitz_is_auto_timezone()) {
		info("[TIMESTAMP] Automatic time update was disabled. We don't need to set time");
		return FALSE;
	}

	/*
	 *  - Apply system time(GMT)
	 */
	info ("[TIMESTAMP][Before] New Time[secs = %lu, nsecs = %lu], Current Time[secs = %lu, nsecs = %lu]",
		newtime.tv_sec, newtime.tv_nsec, curtime.tv_sec, curtime.tv_nsec);

	/* Acquire lock */
	ret = device_power_request_lock(POWER_LOCK_CPU, 0);
	if (ret < 0)
		err("ret : (0x%x)", ret);

	if (alarmmgr_set_systime_with_propagation_delay(newtime, curtime) != ALARMMGR_RESULT_SUCCESS) {
		info ("[TIMESTAMP] alarmmgr_set_systime_with_propagation_delay fail. (ret=%d)", ret);
	} else {
		info ("[TIMESTAMP][After] alarmmgr_set_systime_with_propagation_delay success");
	}

	/* Release lock */
	ret = device_power_release_lock(POWER_LOCK_CPU);
	if (ret < 0)
		err("ret : (0x%x)", ret);
	return TRUE;
}

gboolean nitz_apply_tzfile(const char *tzfilename)
{
	char buf[BUF_SIZE] = {0, };
	char current_tz[BUF_SIZE] = {0, };
	int ret = -1;
	ssize_t len;
	char *current = NULL;

	if (!tzfilename) {
		err("tzfilename is NULL");
		return FALSE;
	}

	current = vconf_get_str(VCONFKEY_TELEPHONY_NITZ_ZONE);
	if (current) {
		if (g_strcmp0(tzfilename, current)) {
			ret = vconf_set_str (VCONFKEY_TELEPHONY_NITZ_ZONE, tzfilename);
		} else {
			dbg("Same timezone - current[%s], tzfilename[%s]", current, tzfilename);
		}
		free(current);
	} else {
		ret = vconf_set_str (VCONFKEY_TELEPHONY_NITZ_ZONE, tzfilename);
	}

	if (!nitz_is_auto_timezone()) {
		info("[TIMESTAMP] Automatic time update was disabled. We don't need to set timezone");
		return FALSE;
	}

	snprintf(buf, BUF_SIZE, "%s/%s", PATH_ZONEINFO, tzfilename);

	len = readlink(PATH_LOCALTIME, current_tz, BUF_SIZE-1);
	if (len < 0) {
		err("Fail to get Current Zone Info");
	} else {
		if (g_strcmp0(current_tz, buf) == 0) {
			info("[TIMESTAMP] We don't need to set timezone again (Already applied)");
			return FALSE;
		} else {
			info("Current TZ:[%s] New TZ:[%s]", current_tz, buf);
		}
	}

	filelog("[TIMESTAMP][Before] timezone will be set");

	/* Acquire lock */
	ret = device_power_request_lock(POWER_LOCK_CPU, 0);
	if (ret < 0)
		err("ret : (0x%x)", ret);

	ret = alarmmgr_set_timezone(buf);

	/* Release lock */
	device_power_release_lock(POWER_LOCK_CPU);
	if (ret < 0)
		err("ret : (0x%x)", ret);

	filelog("set (%s) alarmmgr_set_timezone ret=%d", buf, ret);

	sync();
	tzset();

	filelog("[TIMESTAMP][After] tzname=[%s, %s], timezone=%ld, daylight=%d",
			tzname[0], tzname[1], timezone, daylight);

	if (ret != 0)
		return FALSE;
	else
		return TRUE;
}

static gboolean __update_timezone_by_offset(const struct tnoti_network_timeinfo *ti, char *iso)
{
	char *timezone_name = NULL;
	GList *tz_list = NULL;
	int tz_count = 0;
	gboolean ret = FALSE;
	gboolean found = FALSE;

	tz_list = nitz_get_tzlist_by_offset(iso, ti);
	tz_count = g_list_length(tz_list);
	if (tz_count == 1) {
		timezone_name = tz_list ? tz_list->data : NULL;
		found = timezone_name ? TRUE : FALSE;
		info("Single timezone(%s)", timezone_name);
	} else {
		UChar utf16_timezone[NITZ_TIMEZONE_MAX_LEN] = { 0 };
		UCalendar *cal = NULL;
		gboolean in_dst = FALSE;
		UErrorCode ec = U_ZERO_ERROR;
		GList *list = NULL;

		info("Multi timezone. # of TZ : [%d]", tz_count);
		for (list = tz_list; list != NULL; list = g_list_next(list)) {
			timezone_name = list->data;
			if (strlen(timezone_name) > 0) {
				u_uastrncpy(utf16_timezone, timezone_name, NITZ_TIMEZONE_MAX_LEN);
				cal = ucal_open(utf16_timezone, u_strlen(utf16_timezone), uloc_getDefault(), UCAL_TRADITIONAL, &ec);
				if (cal == NULL) {
					err("ucal_open returns NULL");
					break;
				}
				in_dst = ucal_inDaylightTime(cal, &ec);
				dbg("TZ DST:[%d] WANTED DST:[%d][%d]", in_dst, ti->isdst, ti->dstoff);
				if (ti->dstoff == (int)in_dst) {
					found = TRUE;
					info("(%s) was selected", timezone_name);
					ucal_close(cal);
					break;
				}
				ucal_close(cal);
			}
		}
	}

	if (found) {
		ret = nitz_apply_tzfile (timezone_name);
		nitz_set_iso(iso);
	} else {
		info("[TIMESTAMP] Fail to find timezone");
	}

	g_list_free_full(tz_list, g_free);
	return ret;
}

static gboolean __update_time(const struct tnoti_network_timeinfo *ti,
	struct nitz_custom_data *data, struct timespec curtime)
{
	struct tm tm_time;
	struct timespec newtime = {0,};

	memset(&tm_time, 0, sizeof(struct tm));
	tm_time.tm_year = ti->year - 1900 + 2000;
	tm_time.tm_mon = ti->month - 1;
	tm_time.tm_mday = ti->day;
	tm_time.tm_sec = ti->second;
	tm_time.tm_min = ti->minute;
	tm_time.tm_hour = ti->hour;
	tm_time.tm_wday = ti->wday;
	tm_time.tm_isdst = ti->dstoff;

	newtime.tv_sec = timegm (&tm_time);
	return nitz_set_time(newtime, curtime);
}

static gboolean __update_timezone(const struct tnoti_network_timeinfo *ti,
	struct nitz_custom_data *data)
{
	int mcc = -1;
	char mcc_str[4] = {0,};
	gboolean ret = FALSE;

	memcpy(mcc_str, ti->plmn, 3);
	mcc = atoi(mcc_str);

	/*
	 * Find tzfile
	 */
	if (mcc >= 0) {
		char *iso = __nitz_get_country_code_for_mcc(mcc_str, data);
		ret = __update_timezone_by_offset(ti, iso);
		g_free(iso);
	}

	return ret;
}

GList * nitz_get_tzlist(char *iso)
{
	char* timezone_name = 0;
	UEnumeration* enum_tz = 0;
	UErrorCode ec = U_ZERO_ERROR;
	const UChar* timezone_id = 0;
	int timezone_id_len = 0;
	int offset = 0;
	gboolean in_dst = 0;
	GList *tz_list = NULL;
	UCalendar *cal = NULL;
	unsigned int offset_hash = 0;
	gboolean dup_check[52][2]; /* [offset*2][dst], offset max : GMT+13 */

	enum_tz = ucal_openTimeZoneIDEnumeration(UCAL_ZONE_TYPE_CANONICAL, iso, NULL, &ec);
	memset(dup_check, FALSE, sizeof(dup_check));

	while (1) {
		if (enum_tz == NULL) {
			break;
		}
		timezone_id = uenum_unext(enum_tz, &timezone_id_len, &ec);
		if (timezone_id == NULL)
			break;

		cal = ucal_open(timezone_id, u_strlen(timezone_id), uloc_getDefault(), UCAL_TRADITIONAL, &ec);
		in_dst = ucal_inDaylightTime(cal, &ec);
		offset = ucal_get(cal, UCAL_ZONE_OFFSET, &ec);
		offset_hash = (offset/1800000) + 24;
		ucal_close(cal);

		if (dup_check[offset_hash][in_dst] == TRUE) {
			info("Ignore Duplicated! OFFSET:[%d] DST:[%d] Hash:[%d]", offset, in_dst, offset_hash);
			continue;
		}
		dup_check[offset_hash][in_dst] = TRUE;

		timezone_name = (char *)g_malloc0(NITZ_TIMEZONE_MAX_LEN);
		u_UCharsToChars(timezone_id, timezone_name, NITZ_TIMEZONE_MAX_LEN);
		tz_list = g_list_append(tz_list, timezone_name);
		info("[TIMESTAMP] ISO:[%s] OFFSET:[%d] DST:[%d] TZ:[%s] Hash:[%d]", iso, offset, in_dst, timezone_name, offset_hash);
	}
	uenum_close(enum_tz);
	return tz_list;
}

GList *nitz_get_tzlist_by_offset(char *iso, const struct tnoti_network_timeinfo *ti)
{
	char *timezone_name = 0;
	UEnumeration *enum_tz = 0;
	UErrorCode ec = U_ZERO_ERROR;
	const UChar *timezone_id = 0;
	int timezone_id_len = 0;
	int offset = 0;
	GList *tz_list = NULL;

	if (ti) {
		offset = ti->gmtoff * 60 * 1000;
		if (ti->isdst)
			offset -= ti->dstoff * 60 * 60 * 1000;
		dbg("offset = %d", offset);
		enum_tz = ucal_openTimeZoneIDEnumeration(UCAL_ZONE_TYPE_CANONICAL, iso, &offset, &ec);
	} else {
		err("Invalid offset");
		return NULL;
	}

	while (1) {
		if (enum_tz == NULL)
			break;

		timezone_id = uenum_unext(enum_tz, &timezone_id_len, &ec);
		if (timezone_id == NULL)
			break;

		timezone_name = (char *)g_malloc0(NITZ_TIMEZONE_MAX_LEN);
		u_UCharsToChars(timezone_id, timezone_name, NITZ_TIMEZONE_MAX_LEN);
		tz_list = g_list_append(tz_list, timezone_name);
		info("[TIMESTAMP] ISO:[%s] offset:[%d], Available TZ:[%s]", iso, offset, timezone_name);
	}
	uenum_close(enum_tz);
	return tz_list;
}

gboolean nitz_time_update(const struct tnoti_network_timeinfo *time_info,
	struct nitz_custom_data *data, struct timespec curtime)
{
	gboolean ret_time, ret_tz;

	if (time_info->year == 0
			&& time_info->month == 0
			&& time_info->day == 0
			&& time_info->hour == 0
			&& time_info->minute == 0
			&& time_info->second == 0) {
		/*
		 * Update Timezone only
		 */
		return __update_timezone(time_info, data);
	}

	/*
	 * Update (in order) -
	 *	1. Time
	 *	2. Timezone
	 */
	ret_time = __update_time(time_info, data, curtime);
	ret_tz = __update_timezone(time_info, data);

	if (ret_time || ret_tz)
		return TRUE;
	else
		return FALSE;
}