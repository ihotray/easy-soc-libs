/*
 * ecnt_qos.c - Econet QoS library implementation
 *
 * Copyright (C) 2022 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: maxim.menshikov@iopsys.eu
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/klog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/syslog.h>
#include <errno.h>

#ifndef SYSLOG_ACTION_SIZE_BUFFER
#define SYSLOG_ACTION_SIZE_BUFFER   (10)
#endif

#ifndef SYSLOG_ACTION_READ_ALL
#define SYSLOG_ACTION_READ_ALL      (3)
#endif

#include "qos.h"

/* Is queue number inverse relative to queue count? */
#define QDMA_WAN_QUEUE_INVERSE      (1)
/* Total number of rings */
#define QDMA_WAN_RING_COUNT         (8)
/* Total number of queues */
#define QDMA_WAN_QUEUE_COUNT        (8)
/* Path to QDMA WAN counters in procfs */
#define QDMA_WAN_COUNTERS_PATH      "/proc/qdma_wan/counters"
/* Max length of counter name */
#define STAT_COUNTER_NAME_MAX_LEN   (50)
/* Max length of TX queue identifier */
#define TXQ_NAME_MAX_LEN            (50)
/* Max length of read system log */
#define SYSLOG_MAX_LEN              (20 * 1024 * 1024)

/**
 * Find the last occurrence of the @p pattern in @p str
 *
 * @param str       String to find @p pattern in
 * @param pattern   Pattern to find in @p str
 *
 * @return Pointer to the beginning of the last @p pattern occurrence
 */
static char *_strrstr(char *str, char *pattern)
{
	char  *tmp = str;
	char  *result = NULL;
	size_t pattern_len = strlen(pattern);

	while (1) {
		char *anew = strstr(tmp, pattern);

		if (anew == NULL) {
			break;
		}

		result = anew;
		tmp = anew + pattern_len;
	}

	return result;
}

/**
 * Trigger QDMA WAN counter dump
 *
 * @return     @c true on success, @c false on failure
 */
static bool trigger_qdma_wan_counters_dump(void)
{
	char  b;
	FILE *f;

	f = fopen(QDMA_WAN_COUNTERS_PATH, "r");
	if (f == NULL) {
		syslog(LOG_ERR, "failed to open WAN counter trigger: %s", strerror(errno));
		return false;
	}

	fread(&b, 1, sizeof(b), f);
	fclose(f);
	return true;
}

/**
 * Read queue data from system log
 *
 * @param[in]  queue  The queue number
 * @param[out] stats  The pointer to the structure with statistics
 *
 * @return @c 0 on success, any other value on failure.
 */
static int read_from_syslog(int queue,
                            struct qos_stats *stats)
{
	size_t len;
	int    read;
	char  *buf;
	char   txq_name[TXQ_NAME_MAX_LEN];
	size_t ring;

	len = (size_t)klogctl(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);

	/* Limit size to avoid RAM overuse */
	if (len > SYSLOG_MAX_LEN) {
		len = SYSLOG_MAX_LEN;
	}

	buf = malloc(len);
	if (buf == NULL) {
		syslog(LOG_ERR, "failed to allocate buffer for syslog manipulation");
		return -1;
	}

	read = klogctl(SYSLOG_ACTION_READ_ALL, buf, (int)len);
	if (read == -1) {
		syslog(LOG_ERR, "failed to read system log: %s", strerror(errno));
		free(buf);
		return -1;
	}

#if QDMA_WAN_QUEUE_INVERSE
	/* Modes used in qos module have inverse queue numbers */
	if (queue < QDMA_WAN_QUEUE_COUNT && queue != 0)
		queue = QDMA_WAN_QUEUE_COUNT - queue;
#endif

	/* assume that 'read is always less than 'len' by definition */
	buf[read - 1] = '\0';

	for (ring = 0; ring < QDMA_WAN_RING_COUNT; ++ring)
	{
		uint64_t tx_pkts = 0;
		uint64_t tx_bytes = 0;
		uint64_t tx_dropped_pkts = 0;
		uint64_t tx_dropped_bytes = 0;
		char   *tmp;

		sprintf(txq_name,
		        "Ring %" PRIu8 " Queue %" PRIu8,
		        (uint8_t)ring,
		        (uint8_t)queue);
		tmp = _strrstr(buf, txq_name);
		if (tmp == NULL ||
		    sscanf(tmp + strlen(txq_name) + 1,
		           "tx Counts: %" PRIu64 ", tx Bytes: %" PRIu64 ", "
		           "tx drop Counts: %" PRIu64 ", tx drop Bytes: %" PRIu64,
		           &tx_pkts,
		           &tx_bytes,
		           &tx_dropped_pkts,
		           &tx_dropped_bytes) != 4)
		{
			tx_pkts = 0;
			tx_bytes = 0;
			tx_dropped_pkts = 0;
			tx_dropped_bytes = 0;
		}

		stats->tx_packets += tx_pkts;
		stats->tx_bytes += tx_bytes;
		stats->tx_dropped_packets += tx_dropped_pkts;
		stats->tx_dropped_bytes += tx_dropped_bytes;
	}

	free(buf);
	return 0;
}

/**
 *  Get queue stats on Econet platform
 *  @param [in]  ifname 				input parameter for linux interface
 *  @param [in]  qid 					input parameter for queue id
 *  @param [out] qstats 				output parameter pointer to qos_stats
 *  @param [out] is_read_and_reset 		output parameter for stat fetch was read
 *                                      and reset for driver
 */
static int ecnt_get_stats(const char *ifname,
                          int queue_id,
                          struct qos_stats *stats,
                          int *is_read_and_reset)
{
	/* Ignore interfaces that don't exist */
	if (if_nametoindex(ifname) == 0) {
		syslog(LOG_ERR, "interface doesn't exist: %s", ifname);
		return errno;
	}

	if (!trigger_qdma_wan_counters_dump())
		return -1;

	*is_read_and_reset = 0;

	return read_from_syslog(queue_id, stats);
}

const struct qos_ops qos_ecnt_ops_eth = {
	.ifname = "eth",
	.get_stats = ecnt_get_stats,
};

const struct qos_ops qos_ecnt_ops_nas = {
	.ifname = "nas",
	.get_stats = ecnt_get_stats,
};

const struct qos_ops qos_ecnt_ops_ae_wan = {
	.ifname = "ae_wan",
	.get_stats = ecnt_get_stats,
};
