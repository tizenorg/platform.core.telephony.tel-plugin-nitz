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

#ifndef __NITZ_CITYLIST_H__
#define __NITZ_CITYLIST_H__

/*
 * http://en.wikipedia.org/wiki/List_of_mobile_country_codes
 * http://worldtimeengine.com
 */

typedef struct {
	int mcc;
	char *country;	/* iso3166 */
	int id;		/* id for city name (multi timezone in one country) */
	int std_timezone;
	int dst;
	char *city;	/* timezone city filename */
} NITZ_MCC_TZFILE_MAP;

NITZ_MCC_TZFILE_MAP *nitz_find_tzinfo(int mcc, int gmtoff, int dstoff, int isdst);
int nitz_self_check_tzfile(void);

#endif
