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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <tcore.h>

#include "common.h"
#include "citylist.h"

static NITZ_MCC_TZFILE_MAP nitz_table_mcc_tzfile[] =
{
	{ 202,  "GR", -1, +2, +1, "Europe/Athens" },	// Greece -- Athens
	{ 204,  "NL", -1, +1, +1, "Europe/Amsterdam" },	// Netherlands -- Amsterdam
	{ 206,  "BE", -1, +1, +1, "Europe/Brussels" },	// Belgium -- Brussels
	{ 208,  "FR", -1, +1, +1, "Europe/Paris" },	// France -- Paris
	{ 212,  "MC", -1, +1, +1, "Europe/Monaco" },	// Monaco -- Monaco
	{ 213,  "AD", -1, +1, +1, "Europe/Andorra" },	// Andorra -- Andorra la Vella

	{ 214,  "ES",  0, +1, +1, "Europe/Madrid" },	// Spain -- Madrid
	{ 214,  "ES",  1, +1, +1, "Africa/Ceuta" },	//
	{ 214,  "ES",  2,  0, +1, "Atlantic/Canary" },	//

	{ 216,  "HU", -1, 0, 0, "Europe/Budapest" },	// Hungary -- Budapest
	{ 218,  "BA", -1, 0, 0, "Europe/Sarajevo" },	// Bosnia and Herzegovina -- Sarajevo
	{ 219,  "HR", -1, 0, 0, "Europe/Zagreb" },	// Croatia -- Zagreb
	{ 220,  "RS", -1, 0, 0, "Europe/Belgrade" },	// Serbia -- Belgrade
	{ 222,  "IT", -1, 0, 0, "Europe/Rome" },	// Italy -- Rome
	{ 225,  "VA", -1, 0, 0, "Europe/Vatican" },
	{ 226,  "RO", -1, 0, 0, "Europe/Bucharest" },	// Romania -- Bucharest
	{ 228,  "CH", -1, 0, 0, "Europe/Zurich" },	// Switzerland -- Bern
	{ 230,  "CZ", -1, 0, 0, "Europe/Prague" },	// Czech Republic -- Prague
	{ 231,  "SK", -1, 0, 0, "Europe/Bratislava" },	// Slovak Republic -- Bratislava
	{ 232,  "AT", -1, 0, 0, "Europe/Vienna" },	// Austria -- Vienna
	{ 234,  "GB", -1, 0, 0, "Europe/London" },	// United Kingdom -- London
	{ 235,  "GB", -1, 0, 0, "Europe/London" },	// United Kingdom -- London
	{ 238,  "DK", -1, 0, 0, "Europe/Copenhagen" },	// Denmark -- Copenhagen
	{ 240,  "SE", -1, 0, 0, "Europe/Stockholm" },	// Sweden -- Stockholm
	{ 242,  "NO", -1, 0, 0, "Europe/Oslo" },	// Norway -- Oslo
	{ 244,  "FI", -1, 0, 0, "Europe/Helsinki" },	// Finland -- Helsinki
	{ 246,  "LT", -1, 0, 0, "Europe/Vilnius" },	// Lithuania -- Vilnius
	{ 247,  "LV", -1, 0, 0, "Europe/Riga" },	// Latvia -- Riga
	{ 248,  "EE", -1, 0, 0, "Europe/Tallinn" },	// Estonia -- Tallinn

	{ 250,  "RU",  0,  +3, +1, "Europe/Moscow" },		// Russia -- Moscow+00
	{ 250,  "RU",  1,  +2, +1, "Europe/Kaliningrad" },	// Russia -- Moscow-01
	{ 250,  "RU",  2,  +3, +1, "Europe/Volgograd" },	// Russia -- Moscow+00
	{ 250,  "RU",  3,  +3, +1, "Europe/Samara" },		// Russia -- Moscow
	{ 250,  "RU",  4,  +5, +1, "Asia/Yekaterinburg" },	// Russia -- Moscow+02
	{ 250,  "RU",  5,  +6, +1, "Asia/Omsk" },		// Russia -- Moscow+03
	{ 250,  "RU",  6,  +6, +1, "Asia/Novosibirsk" },	// Russia -- Moscow+03
	{ 250,  "RU",  7,  +6, +1, "Asia/Novokuznetsk" },	// Russia -- Moscow+03
	{ 250,  "RU",  8,  +7, +1, "Asia/Krasnoyarsk" },	// Russia -- Moscow+04
	{ 250,  "RU",  9,  +8, +1, "Asia/Irkutsk" },		// Russia -- Moscow+05
	{ 250,  "RU", 10,  +9, +1, "Asia/Yakutsk" },		// Russia -- Moscow+06
	{ 250,  "RU", 11, +10, +1, "Asia/Vladivostok" },	// Russia -- Moscow+07
	{ 250,  "RU", 12, +10, +1, "Asia/Sakhalin" },		// Russia -- Moscow+07
	{ 250,  "RU", 13, +11, +1, "Asia/Magadan" },		// Russia -- Moscow+08
	{ 250,  "RU", 14, +11, +1, "Asia/Kamchatka" },		// Russia -- Moscow+08
	{ 250,  "RU", 15, +11, +1, "Asia/Anadyr" },		// Russia -- Moscow+08

	{ 255,  "UA",  0, +2, +1, "Europe/Kiev" },		// Ukraine -- Kiev
	{ 255,  "UA",  1, +2, +1, "Europe/Uzhgorod" },		// Ukraine --
	{ 255,  "UA",  2, +2, +1, "Europe/Zaporozhye" },	// Ukraine --
	{ 255,  "UA",  3, +2, +1, "Europe/Simferopol" },	// Ukraine --

	{ 257,  "BY", -1, 0, 0, "Europe/Minsk" },		// Belarus Republic -- Minsk
	{ 259,  "MD", -1, 0, 0, "Europe/Chisinau" },		// Moldova -- Kishinev(Chisinau)
	{ 260,  "PL", -1, 0, 0, "Europe/Warsaw" },		// Poland -- Warsaw
	{ 262,  "DE", -1, 0, 0, "Europe/Berlin" },		// Germany -- Berlin
	{ 266,  "GI", -1, 0, 0, "Europe/Gibraltar" },		// Gibraltar -- Gibraltar

	{ 268,  "PT",  0, +0, +1, "Europe/Lisbon" },	// Portugal -- Lisbon
	{ 268,  "PT",  1, +0, +1, "Atlantic/Madeira" },	//
	{ 268,  "PT",  2, -1, +1, "Atlantic/Azores" },	//

	{ 270,  "LU", -1, 0, 0, "Europe/Luxembourg" },	// Luxembourg -- Luxembourg
	{ 272,  "IE", -1, 0, 0, "Europe/Dublin" },	// Ireland -- Dublin
	{ 274,  "IS", -1, 0, 0, "Atlantic/Reykjavik" },	// Iceland -- Reykjavik
	{ 276,  "AL", -1, 0, 0, "Europe/Tirane" },	// Albania -- Tirane
	{ 278,  "MT", -1, 0, 0, "Europe/Malta" },	// Malta -- Valletta
	{ 280,  "CY", -1, 0, 0, "Asia/Nicosia" },	// Cyprus -- Nicosia
	{ 282,  "GE", -1, 0, 0, "Asia/Tbilisi" },	// Georgia -- Tbilisi
	{ 283,  "AM", -1, 0, 0, "Asia/Yerevan" },	// Armenia -- Yerevan
	{ 284,  "BG", -1, 0, 0, "Europe/Sofia" },	// Bulgaria -- Sofia
	{ 286,  "TR", -1, 0, 0, "Europe/Istanbul" },	// Turkey -- Ankara
	{ 288,  "FO", -1, 0, 0, "Atlantic/Faroe" },	// Faroe Islands -- Torshavn

	{ 290,  "GL",  0, -3, +1, "America/Godthab" },	// Greenland -- Nuuk								/*!*/
	{ 290,  "GL",  1, +0, +0, "America/Danmarkshavn" },	//
	{ 290,  "GL",  2, -1, +1, "America/Scoresbysund" },	//
	{ 290,  "GL",  3, -4, +1, "America/Thule" },		//

	{ 292,  "SM", -1, 0, 0, "Europe/San_Marino" },	// San Marino -- San Marino
	{ 293,  "SI", -1, 0, 0, "Europe/Ljubljana" },	// Slovenia -- Ljubljana
	{ 294,  "MK", -1, 0, 0, "Europe/Skopje" },	// Macedonia -- Skopje
	{ 295,  "LI", -1, 0, 0, "Europe/Vaduz" },	// Liechtenstein -- Vaduz
	{ 297,  "ME", -1, 0, 0, "Europe/Podgorica" },	// Montenegro -- Podgorica

	{ 302,  "CA",  0, -4, +1, "America/St_Johns" },		//
	{ 302,  "CA",  1, -4, +1, "America/Halifax" },		//
	{ 302,  "CA",  2, -4, +1, "America/Glace_Bay" },	//
	{ 302,  "CA",  3, -4, +1, "America/Moncton" },		//
	{ 302,  "CA",  4, -4, +1, "America/Goose_Bay" },	//
	{ 302,  "CA",  5, -4, +0, "America/Blanc-Sablon" },	//
	{ 302,  "CA",  6, -5, +1, "America/Montreal" },		//
	{ 302,  "CA",  7, -5, +1, "America/Toronto" },		//Canada -- Ottawa									/*!*/
	{ 302,  "CA",  8, -5, +1, "America/Nipigon" },		//
	{ 302,  "CA",  9, -5, +1, "America/Thunder_Bay" },	//
	{ 302,  "CA", 10, -5, +1, "America/Iqaluit" },		//
	{ 302,  "CA", 11, -5, +1, "America/Pangnirtung" },	//
	{ 302,  "CA", 12, -6, +1, "America/Resolute" },		//
	{ 302,  "CA", 13, -6, +1, "America/Atikokan" },		//
	{ 302,  "CA", 14, -6, +1, "America/Rankin_Inlet" },	//
	{ 302,  "CA", 15, -6, +1, "America/Winnipeg" },		//
	{ 302,  "CA", 16, -6, +1, "America/Rainy_River" },	//
	{ 302,  "CA", 17, -6, +0, "America/Regina" },		//
	{ 302,  "CA", 18, -8, +1, "America/Swift_Current" },	//
	{ 302,  "CA", 19, -7, +1, "America/Edmonton" },		//
	{ 302,  "CA", 20, -7, +1, "America/Cambridge_Bay" },	//
	{ 302,  "CA", 21, -7, +1, "America/Yellowknife" },	//
	{ 302,  "CA", 22, -7, +1, "America/Inuvik" },		//
	{ 302,  "CA", 23, -7, +0, "America/Dawson_Creek" },	//
	{ 302,  "CA", 24, -8, +1, "America/Vancouver" },	//
	{ 302,  "CA", 25, -8, +1, "America/Whitehorse" },	//
	{ 302,  "CA", 26, -8, +1, "America/Dawson" },		//

	{ 308,  "PM", -1, 0, 0, "America/Miquelon" },	// Saint Pierre and Miquelon -- Saint-Pierre

	{ 310,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 310,  "US",  1,  -5, +1, "America/Detroit" },			//
	{ 310,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 310,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 310,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 310,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 310,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 310,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 310,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 310,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 310,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 310,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 310,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 310,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 310,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 310,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 310,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 310,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 310,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 310,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 310,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 310,  "US", 21,  -9, +1, "America/Anchorage" },	// = Alaska
	{ 310,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 310,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 310,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 310,  "US", 25, -10, +1, "America/Adak" },		//
	{ 310,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 311,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 311,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 311,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 311,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 311,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 311,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 311,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 311,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 311,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 311,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 311,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 311,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 311,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 311,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 311,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 311,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 311,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 311,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 311,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 311,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 311,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 311,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 311,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 311,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 311,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 311,  "US", 25, -10, +1, "America/Adak" },		//
	{ 311,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 312,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 312,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 312,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 312,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 312,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 312,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 312,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 312,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 312,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 312,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 312,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 312,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 312,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 312,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 312,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 312,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 312,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 312,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 312,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 312,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 312,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 312,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 312,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 312,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 312,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 312,  "US", 25, -10, +1, "America/Adak" },		//
	{ 312,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 313,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 313,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 313,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 313,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 313,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 313,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 313,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 313,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 313,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 313,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 313,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 313,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 313,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 313,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 313,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 313,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 313,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 313,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 313,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 313,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 313,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 313,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 313,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 313,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 313,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 313,  "US", 25, -10, +1, "America/Adak" },		//
	{ 313,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 314,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 314,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 314,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 314,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 314,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 314,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 314,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 314,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 314,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 314,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 314,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 314,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 314,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 314,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 314,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 314,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 314,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 314,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 314,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 314,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 314,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 314,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 314,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 314,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 314,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 314,  "US", 25, -10, +1, "America/Adak" },		//
	{ 314,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 315,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 315,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 315,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 315,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 315,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 315,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 315,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 315,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 315,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 315,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 315,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 315,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 315,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 315,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 315,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 315,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 315,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 315,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 315,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 315,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 315,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 315,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 315,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 315,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 315,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 315,  "US", 25, -10, +1, "America/Adak" },		//
	{ 315,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 316,  "US",  0,  -5, +1, "America/New_York" },	// U.S.A											/*!*/
	{ 316,  "US",  1,  -5, +1, "America/Detroit" },		//
	{ 316,  "US",  2,  -5, +1, "America/Kentucky/Louisville" },	//
	{ 316,  "US",  3,  -5, +1, "America/Kentucky/Monticello" },	//
	{ 316,  "US",  4,  -5, +1, "America/Indiana/Indianapolis" },	//
	{ 316,  "US",  5,  -5, +1, "America/Indiana/Vincennes" },	//
	{ 316,  "US",  6,  -5, +1, "America/Indiana/Winamac" },		//
	{ 316,  "US",  7,  -5, +1, "America/Indiana/Marengo" },		//
	{ 316,  "US",  8,  -5, +1, "America/Indiana/Petersburg" },	//
	{ 316,  "US",  9,  -5, +1, "America/Indiana/Vevay" },		//
	{ 316,  "US", 10,  -6, +1, "America/Chicago" },			//
	{ 316,  "US", 11,  -6, +1, "America/Indiana/Tell_City" },	//
	{ 316,  "US", 12,  -6, +1, "America/Indiana/Knox" }, 		//
	{ 316,  "US", 13,  -6, +1, "America/Menominee" },		//
	{ 316,  "US", 14,  -6, +1, "America/North_Dakota/Center" },	//
	{ 316,  "US", 15,  -6, +1, "America/North_Dakota/New_Salem" },	//
	{ 316,  "US", 16,  -7, +1, "America/Denver" },		//
	{ 316,  "US", 17,  -7, +1, "America/Boise" },		//
	{ 316,  "US", 18,  -7, +1, "America/Shiprock" },	//
	{ 316,  "US", 19,  -7, +0, "America/Phoenix" },		//
	{ 316,  "US", 20,  -8, +1, "America/Los_Angeles" },	//
	{ 316,  "US", 21,  -9, +1, "America/Anchorage" },	//
	{ 316,  "US", 22,  -9, +1, "America/Juneau" },		//
	{ 316,  "US", 23,  -9, +1, "America/Yakutat" },		//
	{ 316,  "US", 24,  -9, +1, "America/Nome" },		//
	{ 316,  "US", 25, -10, +1, "America/Adak" },		//
	{ 316,  "US", 26, -10, +0, "Pacific/Honolulu" },	//

	{ 330,  "PR", -1, 0, 0, "America/Puerto_Rico" },	// Puerto Rico -- San Juan
	{ 332,  "VI", -1, 0, 0, "America/St_Thomas" },		// United States Virgin Islands -- Charlotte Amalie	/*!*/

	{ 334,  "MX",  0, -6, +1, "America/Mexico_City" },	// Mexico -- Mexico City
	{ 334,  "MX",  1, -6, +1, "America/Cancun" },		//
	{ 334,  "MX",  2, -6, +1, "America/Merida" },		//
	{ 334,  "MX",  3, -6, +1, "America/Monterrey" },	//
	{ 334,  "MX",  4, -6, +1, "America/Matamoros" },	//
	{ 334,  "MX",  5, -7, +1, "America/Mazatlan" },		//
	{ 334,  "MX",  6, -7, +1, "America/Chihuahua" },	//
	{ 334,  "MX",  7, -6, +1, "America/Ojinaga" },		//
	{ 334,  "MX",  8, -7, +0, "America/Hermosillo" },	//
	{ 334,  "MX",  9, -8, +1, "America/Tijuana" },		//
	{ 334,  "MX", 10, -6, +1, "America/Santa_Isabel" }, 	//
	{ 334,  "MX", 11, -7, +1, "America/Bahia_Banderas" },	//

	{ 338,  "JM", -1, 0, 0, "America/Jamaica" },	// Jamaica -- Kingston
	{ 340,  "GP", -1, 0, 0, "America/Guadeloupe" },	// Guadeloupe(france) -- Basse-Terre
	{ 340,  "MQ", -1, 0, 0, "America/Martinique" },
	{ 342,  "BB", -1, 0, 0, "America/Barbados" },	// Barbados -- Bridgetown
	{ 344,  "AG", -1, 0, 0, "America/Antigua" },	// Antigua & Barbuda -- Saint John's
	{ 346,  "KY", -1, 0, 0, "America/Cayman" },	// Cayman Islands -- George Town
	{ 348,  "VG", -1, 0, 0, "America/Tortola" },	// British Virgin Islands -- Road Town
	{ 350,  "BM", -1, 0, 0, "Atlantic/Bermuda" },	// Bermuda -- Hamilton
	{ 352,  "GD", -1, 0, 0, "America/Grenada" },	// Grenada -- Saint George's
	{ 354,  "MS", -1, 0, 0, "America/Montserrat" },	// Montserrat(UK) -- Plymouth
	{ 356,  "KN", -1, 0, 0, "America/St_Kitts" },	// Saint Kitts and Nevis -- Basseterre
	{ 358,  "LC", -1, 0, 0, "America/St_Lucia" },	// Saint Lucia -- Castries
	{ 360,  "VC", -1, 0, 0, "America/St_Vincent" },	// Saint Vincent and the Grenadines -- Kingstown
	{ 362,  "AN", -1, 0, 0, "America/Curacao" },	// Netherlands Antilles -- Willemstad
	{ 363,  "AW", -1, 0, 0, "America/Aruba" },	// Aruba(netherlands) -- Oranjestad
	{ 364,  "BS", -1, 0, 0, "America/Nassau" },	// Bahamas -- Nassau
	{ 365,  "AI", -1, 0, 0, "America/Antigua" },	// Anguilla -- The Valley
	{ 366,  "DM", -1, 0, 0, "America/Dominica" },	// Dominica -- Roseau
	{ 368,  "CU", -1, 0, 0, "America/Havana" },		// Cuba -- Havana
	{ 370,  "DO", -1, 0, 0, "America/Santo_Domingo" },	// Dominican Republic -- Santo Domingo
	{ 372,  "HT", -1, 0, 0, "America/Port-au-Prince" },	// Haiti -- Port au Prince //world
	{ 374,  "TT", -1, 0, 0, "America/Port_of_Spain" },	// Trinidad and Tobago -- Port of Spain
	{ 376,  "TC", -1, 0, 0, "America/Grand_Turk" },		// Turks & Caicos islands(UK) -- Cockburn Town
	{ 400,  "AZ", -1, 0, 0, "Asia/Baku" },			// Azerbaijan -- Baku

	{ 401,  "KZ",  0, +6, +0, "Asia/Almaty" },
	{ 401,  "KZ",  1, +5, +0, "Asia/Qyzylorda" },
	{ 401,  "KZ",  2, +5, +0, "Asia/Aqtobe" },
	{ 401,  "KZ",  3, +5, +0, "Asia/Aqtau" },
	{ 401,  "KZ",  4, +5, +0, "Asia/Oral" },

	{ 402,  "BT", -1, 0, 0, "Asia/Thimphu" },	// Bhutan -- Thimphu
	{ 404,  "IN", -1, 0, 0, "Asia/Kolkata" },	// India - New Delhi
	{ 405,  "IN", -1, 0, 0, "Asia/Kolkata" },	// India - New Delhi
	{ 410,  "PK", -1, 0, 0, "Asia/Karachi" },	// Pakistan -- Islamabad
	{ 412,  "AF", -1, 0, 0, "Asia/Kabul" },		// Afghanistan -- Kabul
	{ 413,  "LK", -1, 0, 0, "Asia/Colombo" },	// Sri Lanka -- Colombo
	{ 414,  "MM", -1, 0, 0, "Asia/Rangoon" },	// Myanmar -- Rangoon (Yangon)
	{ 415,  "LB", -1, 0, 0, "Asia/Beirut" },	// Lebanon -- Beirut
	{ 416,  "JO", -1, 0, 0, "Asia/Amman" },		// Jordan -- Amman
	{ 417,  "SY", -1, 0, 0, "Asia/Damascus" },	// Syria -- Damascus
	{ 418,  "IQ", -1, 0, 0, "Asia/Baghdad" },	// Iraq  -- Baghdad
	{ 419,  "KW", -1, 0, 0, "Asia/Kuwait" },	// Kuwait -- Kuwait City
	{ 420,  "SA", -1, 0, 0, "Asia/Riyadh" },	// Saudi Arabia -- Riyadh
	{ 421,  "YE", -1, 0, 0, "Asia/Aden" },		// Yemen -- Sanaa		/*!*/
	{ 422,  "OM", -1, 0, 0, "Asia/Muscat" },	// Oman -- Muscat
	{ 423,  "PS", -1, 0, 0, "Asia/Gaza" },
	{ 424,  "AE", -1, 0, 0, "Asia/Dubai" },		// United Arab Emirates -- Abu Dhabi	/*!*/
	{ 425,  "IL", -1, 0, 0, "Asia/Jerusalem" },	// Israel -- Tel Aviv			/*!*/
	{ 426,  "BH", -1, 0, 0, "Asia/Bahrain" },	// Bahrain -- Manama
	{ 427,  "QA", -1, 0, 0, "Asia/Qatar" },		// Qatar -- Doha

	{ 428,  "MN",  0, +8, +0, "Asia/Ulaanbaatar" },	// Mongolia -- Ulaanbaatar
	{ 428,  "MN",  1, +7, +0, "Asia/Hovd" },	//
	{ 428,  "MN",  2, +8, +0, "Asia/Choibalsan" },	//

	{ 429,  "NP", -1, 0, 0, "Asia/Kathmandu" },	// Nepal -- Kathmandu
	{ 430,  "AE", -1, 0, 0, "Asia/Dubai" },		// Abu Dhabi
	{ 431,  "AE", -1, 0, 0, "Asia/Dubai" },		// Dubai
	{ 432,  "IR", -1, 0, 0, "Asia/Tehran" },	// Iran -- Tehran

	{ 434,  "UZ",  0, +5, +0, "Asia/Samarkand" },	// Uzbekistan -- Samarkand
	{ 434,  "UZ",  1, +5, +0, "Asia/Tashkent" },	// Uzbekistan -- Tashkent

	{ 436,  "TJ", -1, 0, 0, "Asia/Dushanbe" },	// Tajikistan -- Dushanbe
	{ 437,  "KG", -1, 0, 0, "Asia/Bishkek" },	// Kyrgyzstan -- Bishkek
	{ 438,  "TM", -1, 0, 0, "Asia/Ashgabat" },	// Turkmenistan -- Ashgabat
	{ 440,  "JP", -1, 0, 0, "Asia/Tokyo" },		// Japan -- Tokyo
	{ 441,  "JP", -1, 0, 0, "Asia/Tokyo" },
	{ 450,  "KR", -1, 0, 0, "Asia/Seoul" },		// South Korea -- Seoul
	{ 452,  "VN", -1, 0, 0, "Asia/Ho_Chi_Minh" },	// Vietnam -- Hanoi
	{ 454,  "HK", -1, 0, 0, "Asia/Hong_Kong" },	// Hong Kong - Hong Kong
	{ 455,  "MO", -1, 0, 0, "Asia/Macau" },		// Macau(PRC) -- ???
	{ 456,  "KH", -1, 0, 0, "Asia/Phnom_Penh" },	// Cambodia -- Phnom Penh
	{ 457,  "LA", -1, 0, 0, "Asia/Vientiane" },	// Laos -- Vientiane

	{ 460,  "CN",  0, +8, +0, "Asia/Shanghai" },	// China -- Beijing	/*!*/
	{ 460,  "CN",  1, +8, +0, "Asia/Harbin" },	//
	{ 460,  "CN",  2, +8, +0, "Asia/Chongqing" },	//
	{ 460,  "CN",  3, +8, +0, "Asia/Urumqi" },	//
	{ 460,  "CN",  4, +8, +0, "Asia/Kashgar" },	//

	{ 466,  "TW", -1, 0, 0, "Asia/Taipei" },	// Taiwan -- Taipei
	{ 467,  "KP", -1, 0, 0, "Asia/Pyongyang" },	// North Korea -- Pyongyang
	{ 470,  "BD", -1, 0, 0, "Asia/Dhaka" },		// Bangladesh -- Dhaka
	{ 472,  "MV", -1, 0, 0, "Indian/Maldives" },	// Maldives -- Male

	{ 502,  "MY",  0, +8, +0, "Asia/Kuala_Lumpur" },// Malaysia -- Kuala Lumpur
	{ 502,  "MY",  1, +8, +0, "Asia/Kuching" },		//

	{ 505,  "AU",  0, +10, +1, "Australia/Sydney" },	// Australia -- Canberra (n-project: Sydney)	/*!*/
	{ 505,  "AU",  1, +10, +1, "Australia/Lord_Howe" },	// +10:30, +00:30				/*!*/
	{ 505,  "AU",  2, +10, +1, "Australia/Hobart" },	//
	{ 505,  "AU",  3, +10, +1, "Australia/Currie" },	//
	{ 505,  "AU",  4, +10, +1, "Australia/Melbourne" },	//
	{ 505,  "AU",  5, +10, +1, "Australia/Broken_Hill" },	//
	{ 505,  "AU",  6, +10, +0, "Australia/Brisbane" },	//
	{ 505,  "AU",  7, +10, +0, "Australia/Lindeman" },	//
	{ 505,  "AU",  8,  +9, +1, "Australia/Adelaide" },	// +09:30, +01:00
	{ 505,  "AU",  9,  +9, +0, "Australia/Darwin" },	// +09:30, +0
	{ 505,  "AU", 10,  +8, +0, "Australia/Perth" },		//
	{ 505,  "AU", 11,  +8, +0, "Australia/Eucla" },		// +08:45

	{ 510,  "ID",  0, +7, +0, "Asia/Jakarta" },	// Indonesia -- Jakarta
	{ 510,  "ID",  1, +7, +0, "Asia/Pontianak" },	//
	{ 510,  "ID",  2, +8, +0, "Asia/Makassar" },	//
	{ 510,  "ID",  3, +9, +0, "Asia/Jayapura" },	//

	{ 514,  "TL", -1, 0, 0, "Asia/Dili" },		// Timor -- Dili
	{ 515,  "PH", -1, 0, 0, "Asia/Manila" },	// Philippines -- Manila
	{ 520,  "TH", -1, 0, 0, "Asia/Bangkok" },	// Thailand -- Bangkok
	{ 525,  "SG", -1, 0, 0, "Asia/Singapore" },	// Singapore -- Singapore
	{ 528,  "BN", -1, 0, 0, "Asia/Brunei" },	// Brunei Darussalam -- Bandar Seri Begawan

	{ 530,  "NZ",  0, +12, +1, "Pacific/Auckland" },// New Zealand -- Wellington	/*!*/
	{ 530,  "NZ",  1, +12, +1, "Pacific/Chatham" },	// +12:45, +01:00

	{ 534,  "MP", -1, 0, 0, "Pacific/Saipan" },	// Northern Mariana Islands (US)
	{ 535,  "GU", -1, 0, 0, "Pacific/Guam" },	// Guam (US)
	{ 536,  "NR", -1, 0, 0, "Pacific/Nauru" },	// Nauru
	{ 537,  "PG", -1, 0, 0, "Pacific/Port_Moresby" }, // Papua New Guinea -- Port Moresby
	{ 539,  "TO", -1, 0, 0, "Pacific/Tongatapu" },	// Tonga -- Nuku'alofa			/*!*/
	{ 540,  "SB", -1, 0, 0, "Pacific/Guadalcanal" },// Solomon -- Honiara			/*!*/
	{ 541,  "VU", -1, 0, 0, "Pacific/Efate" },	// Vanuatu -- Port Vila			/*!*/
	{ 542,  "FJ", -1, 0, 0, "Pacific/Fiji" },	// Fiji -- Suva
	{ 543,  "WF", -1, 0, 0, "Pacific/Wallis" },	// Wallis and Futuna (France)
	{ 544,  "AS", -1, 0, 0, "Pacific/Pago_Pago" },	// American Samoa(US) -- Pago Pago

	{ 545,  "KI",  0, +12, +0, "Pacific/Tarawa" },		//Kiribati -- Tarawa Atoll
	{ 545,  "KI",  1, +11, +0, "Pacific/Enderbury" },	//
	{ 545,  "KI",  2, +14, +0, "Pacific/Kiritimati" },	//

	{ 546,  "NC", -1, 0, 0, "Pacific/Noumea" },		// New Caledonia(France) -- Noumea

	{ 547,  "PF",  0, -10, +0, "Pacific/Tahiti" },	// French Polynesia(France) -- Papeete
	{ 547,  "PF",  1,  -9, +0, "Pacific/Marquesas" },//
	{ 547,  "PF",  2,  -9, +0, "Pacific/Gambier" },	//

	{ 548,  "CK", -1, 0, 0, "Pacific/Rarotonga" },	// Cook Islands(NZ) -- Avarua	/*!*/
	{ 549,  "WS", -1, 0, 0, "Pacific/Apia" },		// Samoa -- Apia

	{ 550,  "FM",  0, +11, +0, "Pacific/Pohnpei" },	// Micronesia -- Palikir
	{ 550,  "FM",  1, +10, +0, "Pacific/Chuuk" },	// Truk
	{ 550,  "FM",  2, +11, +0, "Pacific/Kosrae" },	//

	{ 551,  "MH",  0, +12, +0, "Pacific/Majuro" },	// Marshall Islands
	{ 551,  "MH",  1, +12, +0, "Pacific/Kwajalein" },//

	{ 552,  "PW", -1, 0, 0, "Pacific/Palau" },	// Palau -- Ngerulmud
	{ 602,  "EG", -1, 0, 0, "Africa/Cairo" },	// Cairo -- Cairo
	{ 603,  "DZ", -1, 0, 0, "Africa/Algiers" },	// Algeri -- Algiers
	{ 604,  "MA", -1, 0, 0, "Africa/Casablanca" },	// Morocco -- Rabat	/*!*/
	{ 605,  "TN", -1, 0, 0, "Africa/Tunis" },	// Tunisia -- Tunis
	{ 606,  "LY", -1, 0, 0, "Africa/Tripoli" },	// Libya -- Tripoli
	{ 607,  "GM", -1, 0, 0, "Africa/Banjul" },	// Gambia -- Banjul
	{ 608,  "SN", -1, 0, 0, "Africa/Dakar" },	// Senegal -- Dakar
	{ 609,  "MR", -1, 0, 0, "Africa/Nouakchott" },	// Mauritania -- Nouakchott
	{ 610,  "ML", -1, 0, 0, "Africa/Bamako" },	// Mali -- Bamako
	{ 611,  "GN", -1, 0, 0, "Africa/Conakry" },	// Guinea -- Conakry
	{ 612,  "CI", -1, 0, 0, "Africa/Abidjan" },	// lvory coast -- Yamoussoukro	/*!*/
	{ 613,  "BF", -1, 0, 0, "Africa/Ouagadougou" },	// Burkina faso -- Ouagadougou
	{ 614,  "NE", -1, 0, 0, "Africa/Niamey" },	// Niger -- Niamey
	{ 615,  "TG", -1, 0, 0, "Africa/Lome" },	// Togo -- Lome
	{ 616,  "BJ", -1, 0, 0, "Africa/Porto-Novo" },	// Benin -- Porto-Novo (n-project: Cotonou)
	{ 617,  "MU", -1, 0, 0, "Indian/Mauritius" },	// Mauritius -- Port Louis (n-project: Plaisance-PlaineMagnien)
	{ 618,  "LR", -1, 0, 0, "Africa/Monrovia" },	// Liberia -- Monrovia
	{ 619,  "SL", -1, 0, 0, "Africa/Freetown" },	// Sierra leone -- Freetown
	{ 620,  "GH", -1, 0, 0, "Africa/Accra" },	// Ghana -- Accra
	{ 621,  "NG", -1, 0, 0, "Africa/Lagos" },	// Nigeria -- Abuja	/*!*/
	{ 622,  "TD", -1, 0, 0, "Africa/Ndjamena" },	// Chad -- N'Djamena
	{ 623,  "CF", -1, 0, 0, "Africa/Bangui" },	// Central african republic -- Bangui
	{ 624,  "CM", -1, 0, 0, "Africa/Douala" },	// Cameroon -- Yaounde			/*!*/
	{ 625,  "CV", -1, 0, 0, "Atlantic/Cape_Verde" },// Cape verde -- Praia (n-project: IlhaDoSal)
	{ 626,  "ST", -1, 0, 0, "Africa/Sao_Tome" },	// Sao tome and principe -- Sao Tome
	{ 627,  "GQ", -1, 0, 0, "Africa/Malabo" },	// Equatorial guinea -- Malabo
	{ 628,  "GA", -1, 0, 0, "Africa/Libreville" },	// Gabon -- Libreville
	{ 629,  "CG", -1, 0, 0, "Africa/Brazzaville" },	// Republic of the congo -- Brazzaville

	{ 630,  "CD",  0, +1, +0, "Africa/Kinshasa" },	// Democratic republic of the congo -- Kinshasa
	{ 630,  "CD",  1, +2, +0, "Africa/Lubumbashi" },//

	{ 631,  "AO", -1, 0, 0, "Africa/Luanda" },	// Angola -- Luanda
	{ 632,  "GW", -1, 0, 0, "Africa/Bissau" },	// Guinea-bissau -- Bissau
	{ 633,  "SC", -1, 0, 0, "Indian/Mahe" },	// Seychelles -- Victoria	/*!*/
	{ 634,  "SD", -1, 0, 0, "Africa/Khartoum" },	// Sudan -- Khartoum
	{ 635,  "RW", -1, 0, 0, "Africa/Kigali" },	// Rwanda -- Kigali
	{ 636,  "ET", -1, 0, 0, "Africa/Addis_Ababa" },	// Ethiopia -- Addis Ababa
	{ 637,  "SO", -1, 0, 0, "Africa/Mogadishu" },	// Somalia -- Mogadishu
	{ 638,  "DJ", -1, 0, 0, "Africa/Djibouti" },	// Djibouti -- Djibouti
	{ 639,  "KE", -1, 0, 0, "Africa/Nairobi" },	// Kenya -- Nairobi
	{ 640,  "TZ", -1, 0, 0, "Africa/Dar_es_Salaam" }, // Tanzania -- Dar es Salaam
	{ 641,  "UG", -1, 0, 0, "Africa/Kampala" },	// Uganda -- Kampala (n-project: Entebbe)
	{ 642,  "BI", -1, 0, 0, "Africa/Bujumbura" },	// Burundi -- Bujumbura
	{ 643,  "MZ", -1, 0, 0, "Africa/Maputo" },	// Mozambique -- Maputo
	{ 645,  "ZM", -1, 0, 0, "Africa/Lusaka" },	// Zambia -- Lusaka
	{ 646,  "MG", -1, 0, 0, "Indian/Antananarivo" },// Madagascar -- Antananarivo
	{ 647,  "RE", -1, 0, 0, "Indian/Reunion" },	// Reunion (France) -- no capital
	{ 648,  "ZW", -1, 0, 0, "Africa/Harare" },	// Zimbabwe -- Harare
	{ 649,  "NA", -1, 0, 0, "Africa/Windhoek" },	// Namibia -- Windhoek
	{ 650,  "MW", -1, 0, 0, "Africa/Blantyre" },	// Malawi -- Lilongwe	/*!*/
	{ 651,  "LS", -1, 0, 0, "Africa/Maseru" },	// Lesotho -- Maseru
	{ 652,  "BW", -1, 0, 0, "Africa/Gaborone" },	// Botswana -- Gaborone
	{ 653,  "SZ", -1, 0, 0, "Africa/Mbabane" },	// Swaziland -- Mbabane
	{ 654,  "KM", -1, 0, 0, "Indian/Comoro" },	// Comoros -- Moroni
	{ 655,  "ZA", -1, 0, 0, "Africa/Johannesburg" },// South africa -- Pretoria (administrative); Cape Town (legislative); Bloemfontein (judiciary)
	{ 657,  "ER", -1, 0, 0, "Africa/Asmara" },	// Eritrea
	{ 702,  "BZ", -1, 0, 0, "America/Belize" },	// Belize -- Belmopan
	{ 704,  "GT", -1, 0, 0, "America/Guatemala" },	// Guatemala -- Guatemala City
	{ 706,  "SV", -1, 0, 0, "America/El_Salvador" },// El salvador -- San Salvador
	{ 708,  "HN", -1, 0, 0, "America/Tegucigalpa" },// Honduras -- Tegucigalpa
	{ 710,  "NI", -1, 0, 0, "America/Managua" },	// Nicaragua -- Managua
	{ 712,  "CR", -1, 0, 0, "America/Costa_Rica" },	// Costa rica -- San Jose
	{ 714,  "PA", -1, 0, 0, "America/Panama" },	// Panama -- Panama City
	{ 716,  "PE", -1, 0, 0, "America/Lima" },	// Peru -- Lima

	{ 722,  "AR",  0, -3, +0, "America/Argentina/Buenos_Aires" },	// Argentina -- Buenos Aires
	{ 722,  "AR",  1, -3, +0, "America/Argentina/Cordoba" },	//
	{ 722,  "AR",  2, -3, +0, "America/Argentina/Salta" },		//
	{ 722,  "AR",  3, -3, +0, "America/Argentina/Jujuy" },		//
	{ 722,  "AR",  4, -3, +0, "America/Argentina/Tucuman" },	//
	{ 722,  "AR",  5, -3, +0, "America/Argentina/Catamarca" },	//
	{ 722,  "AR",  6, -3, +0, "America/Argentina/La_Rioja" },	//
	{ 722,  "AR",  7, -3, +0, "America/Argentina/San_Juan" },	//
	{ 722,  "AR",  8, -3, +0, "America/Argentina/Mendoza" },	//
	{ 722,  "AR",  9, -3, +0, "America/Argentina/San_Luis" },	//
	{ 722,  "AR", 10, -3, +0, "America/Argentina/Rio_Gallegos" },	//
	{ 722,  "AR", 11, -3, +0, "America/Argentina/Ushuaia" },	//

	{ 724,  "BR",  0, -3, +1, "America/Sao_Paulo" },	// Brazil -- Brasilia	/*!*/
	{ 724,  "BR",  1, -2, +0, "America/Noronha" },		//
	{ 724,  "BR",  2, -3, +0, "America/Belem" },		//
	{ 724,  "BR",  3, -3, +0, "America/Fortaleza" },	//
	{ 724,  "BR",  4, -3, +0, "America/Recife" },		//
	{ 724,  "BR",  5, -3, +0, "America/Araguaina" },	//
	{ 724,  "BR",  6, -3, +0, "America/Maceio" },		//
	{ 724,  "BR",  7, -3, +0, "America/Bahia" },		//
	{ 724,  "BR",  8, -4, +1, "America/Campo_Grande" }, 	//
	{ 724,  "BR",  9, -4, +1, "America/Cuiaba" },		//
	{ 724,  "BR", 10, -4, +0, "America/Santarem" },		//
	{ 724,  "BR", 11, -4, +0, "America/Porto_Velho" },	//
	{ 724,  "BR", 12, -4, +0, "America/Boa_Vista" },	//
	{ 724,  "BR", 13, -4, +0, "America/Manaus" },		//
	{ 724,  "BR", 14, -4, +0, "America/Eirunepe" },		//
	{ 724,  "BR", 15, -4, +0, "America/Rio_Branco" },	//

	{ 730,  "CL",  0, -4, +1, "America/Santiago" },	// Chile -- Santiago
	{ 730,  "CL",  1, -6, +1, "Pacific/Easter" },	//

	{ 734,  "VE", -1, 0, 0, "America/Caracas" },	// Venezuela -- Caracas
	{ 736,  "BO", -1, 0, 0, "America/La_Paz" },	// Bolivia -- La Paz (administrative); Sucre (judicial)
	{ 738,  "GY", -1, 0, 0, "America/Guyana" },	// Guyana -- Georgetown

	{ 740,  "EC",  0, -5, +0, "America/Guayaquil" },// Ecuador -- Quito	/*!*/
	{ 740,  "EC",  1, -6, +0, "Pacific/Galapagos" },//

	{ 742,  "GF", -1, 0, 0, "America/Cayenne" },	// French Guiana (France)
	{ 744,  "PY", -1, 0, 0, "America/Asuncion" },	// Paraguay -- Asuncion
	{ 746,  "SR", -1, 0, 0, "America/Paramaribo" },	// Suriname -- Paramaribo
	{ 748,  "UY", -1, 0, 0, "America/Montevideo" },	// Uruguay -- Montevideo
	{ 750,  "FK", -1, 0, 0, "Atlantic/Stanley" },	// Falkland Islands (Malvinas)

	{ -1, NULL, -1, -1, -1, NULL },
};

/*
 * FIXME: need to check exceptional timezone
 */
static NITZ_MCC_TZFILE_MAP *nitz_find_tzinfo_by_mcc_timezone(int mcc, int tz, int dst, int dst_valid)
{
	NITZ_MCC_TZFILE_MAP *t = NULL;
	int calc_tz = 0;

	dbg("Request mcc: [%d], tz: [%d], dst: [%d], dst_valid: [%d]",
						mcc, tz, dst, dst_valid);
	filelog("Find tzfinfo - multi timezone (mcc = %d)", mcc);
	filelog("+- wanted tz: %d,  dst: %d,  dst_valid: %d", tz, dst, dst_valid);

	t = nitz_table_mcc_tzfile;
	while (1) {
		if (t->mcc == -1)
			return NULL;

		if (t->mcc == mcc) {
			calc_tz = t->std_timezone * 60;
			dbg("mcc(%d) matched!! [tz=%d, dst=%d, calc_tz=%d, city=%s]", mcc, t->std_timezone, t->dst, calc_tz, t->city);

			filelog("+- tz: %d (%d), dst: %d, calc_tz: %d, city: %s",
					t->std_timezone, t->std_timezone*60, t->dst, calc_tz, t->city);

			if (dst && dst_valid) {
				if (!t->dst)
					goto CONT;

				calc_tz += t->dst * 60;
			}

			if (calc_tz == tz)
				return t;
		}
CONT:
		t++;
	}

	return NULL;
}

static NITZ_MCC_TZFILE_MAP *nitz_find_tzinfo_by_mcc(int mcc)
{
	NITZ_MCC_TZFILE_MAP *t = NULL;

	if (mcc < 0)
		return NULL;

	t = nitz_table_mcc_tzfile;
	while (1) {
		if (t->mcc == -1)
			return NULL;

		if (t->mcc == mcc)
			return t;

		t++;
	}

	return NULL;
}

static gboolean nitz_is_multi_timezone(NITZ_MCC_TZFILE_MAP *m)
{
	if (m->id == -1)
		return FALSE;

	return TRUE;
}

NITZ_MCC_TZFILE_MAP *nitz_find_tzinfo(int mcc, int gmtoff, int dstoff, int isdst)
{
	NITZ_MCC_TZFILE_MAP *m;

	m = nitz_find_tzinfo_by_mcc(mcc);
	if (nitz_is_multi_timezone(m)) {
		dbg("Multi timezone");
		m = nitz_find_tzinfo_by_mcc_timezone(mcc, gmtoff, dstoff, isdst);
	}

	return m;
}

int nitz_self_check_tzfile(void)
{
	NITZ_MCC_TZFILE_MAP *t = NULL;
	struct stat file_info;
	char buf[255] = {0,};
	int ret = TRUE;

	t = nitz_table_mcc_tzfile;
	while (1) {
		if (t->mcc == -1)
			return ret;

		/* check tzfile */
		snprintf (buf, 255, "/usr/share/zoneinfo/%s", t->city);
		if (stat(buf, &file_info) == -1) {
			filelog("file not exist!! [mcc=%d, tz=%3d, %s]", t->mcc, t->std_timezone, buf);
			ret = FALSE;
		}

		t++;
	}

	return ret;
}
