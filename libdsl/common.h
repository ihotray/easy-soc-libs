/*
 * common.h - Common definitions for xDSL, ATM and PTM
 *
 * Copyright (C) 2020 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: yalu.zhang@iopsys.eu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define LIBDSL_LOG(log_level, format...) fprintf(stderr, ##format)	/* Flawfinder: ignore */

/** enum dsl_status - operational status of a line, channel or an interface */
enum itf_status {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	IF_UP = 1,
	IF_DOWN,
	IF_UNKNOWN,
	IF_DORMANT,
	IF_NOTPRESENT,
	IF_LLDOWN,
	IF_ERROR
};

/** enum diag_state - The diagnostic state of an ATM loopback test */
enum diag_state {
	ATM_DIAG_NONE,
	ATM_DIAG_REQUESTED,
	ATM_DIAG_CANCELED,
	ATM_DIAG_COMPLETE,
	ATM_DIAG_ERROR,
	ATM_DIAG_ERROR_INTERNAL,
	ATM_DIAG_ERROR_OTHER
};

/**
 * Mapping between a string and an enum
 */
struct str_enum_map {
	char *val_str;
	int val_enum;
};

extern const struct str_enum_map vdsl2_profiles[];
extern const struct str_enum_map fast_profiles[];
extern const struct str_enum_map atm_encapsulations[];
extern const struct str_enum_map atm_link_types[];
extern const struct str_enum_map atm_qos_classes[];

int dsl_get_enum_value(const struct str_enum_map *mappings, const char *str_value);
const char *dsl_get_string_value(const struct str_enum_map *mappings, int enum_value);

#ifdef __cplusplus
}
#endif
#endif /* _COMMON_H_ */
