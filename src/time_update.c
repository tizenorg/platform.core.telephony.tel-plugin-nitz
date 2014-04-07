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
#include <sysman.h>

#include "common.h"
#include "time_update.h"
#include "citylist.h"

#define BUF_SIZE 255
#define PATH_LOCALTIME	"/opt/etc/localtime"
#define PATH_ZONEINFO	"/usr/share/zoneinfo"

int nitz_apply_tzfile(const char *tzfilename, gboolean mode_auto)
{
	char buf[BUF_SIZE] = {0, };
	int ret;

	vconf_set_str(VCONFKEY_TELEPHONY_NITZ_ZONE, tzfilename);

	if (mode_auto == FALSE)
		return 0;

	snprintf(buf, BUF_SIZE, "%s/%s", PATH_ZONEINFO, tzfilename);

	ret = sysman_set_timezone(buf);
	filelog("set (%s) timezone file. ret=%d", buf, ret);

	sync();
	tzset();

	filelog("+- Result: tzname=[%s, %s], timezone=%ld, daylight=%d",
				tzname[0], tzname[1], timezone, daylight);

	return ret;
}

long nitz_get_uptime()
{
	struct sysinfo info;

	if (sysinfo(&info) == 0) {
		dbg("uptime: %ld secs", info.uptime);
		return info.uptime;
	}

	return 0;
}

static gboolean update_time(const TelNetworkNitzInfoNoti *ti, gboolean mode_auto)
{
	struct tm tm_time;
	time_t tt_gmt_nitz;
	int ret;

	memset(&tm_time, 0, sizeof(struct tm));
	tm_time.tm_year = ti->year - 1900 + 2000;
	tm_time.tm_mon = ti->month - 1;
	tm_time.tm_mday = ti->day;
	tm_time.tm_sec = ti->second;
	tm_time.tm_min = ti->minute;
	tm_time.tm_hour = ti->hour;
	tm_time.tm_isdst = ti->dstoff;

	tt_gmt_nitz = timegm(&tm_time);
	tt_gmt_nitz -= ti->gmtoff * 60;
	dbg("NITZ GMT Time = %ld", tt_gmt_nitz);

	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_GMT, tt_gmt_nitz);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_EVENT_GMT, nitz_get_uptime());

	if (mode_auto == FALSE) {
		return FALSE;
	}

	/*
	 *  - Apply system time (GMT)
	 */
	ret = sysman_set_datetime(tt_gmt_nitz);
	if (ret < 0) {
		filelog("sysman_set_datetime(%ld) failed. ret = %d",
							tt_gmt_nitz, ret);
		return FALSE;
	} else {
		filelog("sysman_set_datetime(%ld) success. ret = %d",
							tt_gmt_nitz, ret);
	}

	return TRUE;
}

static gboolean update_timezone(const TelNetworkNitzInfoNoti *ti, gboolean mode_auto)
{
	int mcc;
	char mcc_str[4];
	NITZ_MCC_TZFILE_MAP *m;
	gboolean ret = FALSE;

	snprintf(mcc_str, 4, "%s", ti->plmn);
	mcc = atoi(mcc_str);
	dbg("MCC: [%d]", mcc);

	if (mcc > 0) {
		m = nitz_find_tzinfo(mcc, ti->gmtoff, ti->dstoff, ti->isdst);
		if (m) {
			dbg("Country: [%s] (ISO 3166)", m->country);
			dbg("City: [%s]", m->city);
			nitz_apply_tzfile(m->city, mode_auto);
			ret = TRUE;
		} else {
			dbg("No search result");
		}
	}

	return ret;
}

gboolean nitz_time_update(const TelNetworkNitzInfoNoti *time_info, gboolean mode_auto)
{
	if (time_info->year == 0
			&& time_info->month == 0
			&& time_info->day == 0
			&& time_info->hour == 0
			&& time_info->minute == 0
			&& time_info->second == 0) {
		/* Timezone only */
		return update_timezone(time_info, mode_auto);
	}

	update_time(time_info, mode_auto);

	return update_timezone(time_info, mode_auto);
}
