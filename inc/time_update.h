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

#ifndef __NITZ_TIME_H__
#define __NITZ_TIME_H__

int nitz_apply_tzfile (const char *tzfilename);
gboolean nitz_is_auto_timezone (void);
gboolean nitz_is_iso_changed(char *cur_iso);
void nitz_set_iso(char *iso);
gboolean nitz_time_update (const struct tnoti_network_timeinfo *time_info, struct nitz_custom_data *data, struct timespec curtime);
GList *nitz_get_tzlist(char *iso);
GList *nitz_get_tzlist_by_offset(char *iso, const struct tnoti_network_timeinfo *ti);
gboolean nitz_set_time (struct timespec new_time, struct timespec curtime);

#endif
