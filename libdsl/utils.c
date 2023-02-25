/*
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

#include "xdsl.h"
#include "xtm.h"

const struct str_enum_map vdsl2_profiles[] = {
	{ "8a", VDSL2_8a },
	{ "8b", VDSL2_8b },
	{ "8c", VDSL2_8c },
	{ "8d", VDSL2_8d },
	{ "12a", VDSL2_12a },
	{ "12b", VDSL2_12b },
	{ "17a", VDSL2_17a },
	{ "30a", VDSL2_30a },
	{ "35b", VDSL2_35b },
	{ NULL, -1 }
};

const struct str_enum_map fast_profiles[] = {
	{ "106a", FAST_106a },
	{ "106b", FAST_106b },
	{ "106c", FAST_106c },
	{ "212a", FAST_212a },
	{ "212c", FAST_212c },
	{ NULL, -1 }
};

const struct str_enum_map atm_link_types[] = {
	{ "unconfigured", ATM_LINK_UNCONFIGURED },
	{ "eoa", ATM_LINK_EoA },
	{ "ipoa", ATM_LINK_IPoA },
	{ "pppoa", ATM_LINK_PPPoA },
	{ "cip", ATM_LINK_CIP },
	{ NULL, -1 }
};

const struct str_enum_map atm_encapsulations[] = {
	{ "llc",  ATM_LLC },
	{ "vcmux", ATM_VCMUX },
	{ NULL, -1 }
};

const struct str_enum_map atm_qos_classes[] = {
	{ "ubr", ATM_QoS_UBR },
	{ "cbr", ATM_QoS_CBR },
	{ "gfr", ATM_QoS_GFR },
	{ "vbr_nrt", ATM_QoS_VBR_nrt },
	{ "vbr_rt", ATM_QoS_VBR_rt },
	{ "ubr_plus", ATM_QoS_UBR_PLUS },
	{ "abr", ATM_QoS_ABR },
	{ NULL, -1 }
};

/**
	This function converts a string value to the corresponding enum value.

	\param mappings
		The mapping arrays whose element contains a string value and an enum one.
		This parameter must end with { NULL, -1 }.

	\param str_value
		The string value to be converted.

	\return
		Returns 0 on success. Otherwise a negative value is returned.
*/
int dsl_get_enum_value(const struct str_enum_map *mappings, const char *str_value)
{
	const struct str_enum_map *element;

	for (element = mappings; element->val_str != NULL; element++) {
		if (strcasecmp(element->val_str, str_value) == 0)
			return element->val_enum;
	}

	return -1;
}

/**
	This function converts an enum value to the corresponding string.

	\param mappings
		The mapping arrays whose element contains a string value and an enum one.
		This parameter must end with { NULL, -1 }.

	\param enum_value
		The enum value to be converted.

	\return
		Returns the string on success. Otherwise NULL is returned.
*/
const char *dsl_get_string_value(const struct str_enum_map *mappings, int enum_value)
{
	const struct str_enum_map *element;

	for (element = mappings; element->val_str != NULL; element++) {
		if (element->val_enum == enum_value)
			return element->val_str;
	}

	return NULL;
}

int dsl_alloc_ulong_elements(dsl_ulong_elements_t *element, int count)
{
	if (!element || count <= 0)
		return -1;

	element->elements = calloc(count, sizeof(*element->elements));
	if (!element->elements) {
		element->count = 0;
		return -1;
	}

	element->count = count;

	return 0;
}


int dsl_free_ulong_elements(dsl_ulong_elements_t *element)
{
	if (!element)
		return -1;

	if (element->elements) {
		free(element->elements);
		element->elements = NULL;
	}
	element->count = 0;

	return 0;
}
