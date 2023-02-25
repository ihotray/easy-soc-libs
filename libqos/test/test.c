/*
 * test.c - Test module implementation for libqos
 *
 * Copyright (C) 2020 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: oskar.viljasaar@iopsys.eu
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

#include <string.h>

#include "test.h"

static int test_get_stats(const char *ifname,
			 int queue_id, struct qos_stats *stats, int *is_read_and_reset)
{
	memcpy(stats, test_qos_stats, sizeof(struct qos_stats));
	*is_read_and_reset = 1;

	return 0;
}

const struct qos_ops qos_test_ops = {
	.ifname = "eth",
	.get_stats = test_get_stats,
};
