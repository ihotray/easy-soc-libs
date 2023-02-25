/*
 * qos.c - libqos library interface
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

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "easy.h"
#include "qos.h"

/* List of implementations compiled in */
const struct qos_ops *qos_ops[] = {
#ifdef IOPSYS_BROADCOM
	&bcm_rdpa_ops,
#endif
#ifdef IOPSYS_TEST
	&qos_test_ops,
#endif
#ifdef IOPSYS_ECONET
    &qos_ecnt_ops_eth,
    &qos_ecnt_ops_nas,
    &qos_ecnt_ops_ae_wan,
#endif
#if defined(IOPSYS_LINUX) || defined(IPQ95XX)
    &qos_linux_ops_eth,
#endif
};

/** Finds the right driver in function of what interface names it advertises to support */
const struct qos_ops *get_qos_driver(const char *ifname)
{
	int i;

	for (i = 0; i < sizeof(qos_ops)/sizeof(qos_ops[0]); i++) {
		if (!strncmp(qos_ops[i]->ifname, ifname,
					strlen(qos_ops[i]->ifname)))
			return qos_ops[i];
	}

	return NULL;
}

int qos_get_stats(const char *ifname, int queue_id, struct qos_stats *stats, int *is_read_and_reset)
{
	const struct qos_ops *qos = get_qos_driver(ifname);

	if (qos && qos->get_stats)
		return qos->get_stats(ifname, queue_id, stats, is_read_and_reset);

	return -1;
}
