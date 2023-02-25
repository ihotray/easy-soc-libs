/*
 * bcm_qos_archer.c - Broadcom RDPA QoS library implementation
 *
 * Copyright (C) 2021 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: amit.kumar@iopsys.eu
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

#include <archer.h>

#include "qos.h"



/**
 * To get the queue stats from archer device driver
 * Fill up the archer_txq_stats_ioctl_t and read stats
 * from archer device.
 *  @param devType input parameter for device type
 *  @param ifname input parameter for linux interface
 *  @param qid input parameter for queue id
 *  @param qstats output parameter pointer to qos_stats
 * */
int qos_get_queue_stats(int devType, const char* ifname, int qid,
					struct qos_stats *qstats)
{
	archer_txq_stats_ioctl_t stats_ioctl;
	archer_ioctl_cmd_t ioctl_cmd;
	int fd=-1;
	int ret;

	strncpy(stats_ioctl.if_name, ifname, ARCHER_IFNAMSIZ);
	ioctl_cmd = ARCHER_IOC_ENETTXQSTATS_GET;


	stats_ioctl.queue_id = qid;


	fd = open(ARCHER_DRV_DEVICE_NAME, O_RDWR);
	if(fd < 0) {
		syslog(LOG_ERR, "archer device open failed");
		return -1;
	}

	ret = ioctl(fd, ioctl_cmd, &stats_ioctl);
	if(ret)	{
		syslog(LOG_ERR, "ioctl failed for archer");
		close(fd);
		return -1;
	}

	qstats->tx_packets = stats_ioctl.stats.txPackets;
	qstats->tx_bytes = stats_ioctl.stats.txBytes;
	qstats->tx_dropped_packets = stats_ioctl.stats.droppedPackets;
	qstats->tx_dropped_bytes = stats_ioctl.stats.droppedBytes;

	return 0;
}

/**
 *  To get the queue stats for an ARCHER device:
 *  @param ifname input parameter for linux interface
 *  @param qid input parameter for queue id
 *  @param qstats output parameter pointer to qos_stats
 *  @param is_read_and_reset output parameter for stat fetch was read and reset for driver
 */
static int bcm_get_stats(const char *ifname,
			 int queue_id, struct qos_stats *stats, int *is_read_and_reset)
{
	int ret;

	ret = qos_get_queue_stats(0, ifname, queue_id, stats);
	*is_read_and_reset = 0;

	return ret;
}

const struct qos_ops bcm_rdpa_ops = {
	.ifname = "eth",
	.get_stats = bcm_get_stats,
};
