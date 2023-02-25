/*
 * linux_eth.c - Linux based statistics collection ubus methods.
 *
 * Copyright (C) 2023 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: padmalochan.mohapatra@iopsys.eu
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
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include "easy.h"
#include "if_utils.h"
#include "../ethernet.h"

#include "linux_eth.h"

typedef enum {
    tx_packets = 0,
    tx_bytes,
    rx_packets,
    rx_bytes,
    TxDrop,
    TxCrcErr,
    TxUnicast,
    TxMulticast,
    TxBroadcast,
    TxCollision,
    TxSingleCollision,
    TxMultipleCollision,
    TxDeferred,
    TxLateCollision,
    TxExcessiveCollistion,
    TxPause,
    TxPktSz64,
    TxPktSz65To127,
    TxPktSz128To255,
    TxPktSz256To511,
    TxPktSz512To1023,
    Tx1024ToMax,
    TxBytes,
    RxDrop,
    RxFiltering,
    RxUnicast,
    RxMulticast,
    RxBroadcast,
    RxAlignErr,
    RxCrcErr,
    RxUnderSizeErr,
    RxFragErr,
    RxOverSzErr,
    RxJabberErr,
    RxPause,
    RxPktSz64,
    RxPktSz65To127,
    RxPktSz128To255,
    RxPktSz256To511,
    RxPktSz512To1023,
    RxPktSz1024ToMax,
    RxBytes,
    RxCtrlDrop,
    RxIngressDrop,
    RxArlDrop,
    STATS_END,
} idx;

struct ethnic_stats {
    uint32_t   cmd;
    uint32_t   n_stats;
    uint64_t   data[STATS_END];
} stat;

static struct eth_stats ifstats;

static int get_ifstats(const char *ifname, struct eth_stats *s)
{
    struct if_stats easy_ifstat;

    if (!s)
        return -1;

    memset(&easy_ifstat, 0, sizeof(struct if_stats));
    if (if_getstats(ifname, &easy_ifstat) < 0) {
	return -1;
    }

    s->tx_bytes = easy_ifstat.tx_bytes;
    s->tx_packets = easy_ifstat.tx_packets;
    s->tx_errors = easy_ifstat.tx_errors;
/* Some counters set to zero, and will be populated cross referring
 * the rmon stats structure.
 */
    s->tx_ucast_packets = 0;
    s->tx_mcast_packets = 0;
    s->tx_bcast_packets = 0;
    s->tx_discard_packets = 0;
    s->rx_bytes = easy_ifstat.rx_bytes;
    s->rx_packets = 0;
    s->rx_errors = easy_ifstat.rx_errors;
    s->rx_ucast_packets = 0;
    s->rx_mcast_packets = 0;
    s->rx_bcast_packets = 0;
    s->rx_discard_packets = 0;
    s->rx_unknown_packets = 0;

    return 0;
}

int get_ethtool_stats(const char *ifname)
{
    int ret = 0;

    stat.cmd = ETHTOOL_GSTATS;
    if (0 != eth_ioctl(ifname, SIOCETHTOOL, &stat, sizeof(struct ethnic_stats)))
        ret = -1;

    if (ret)
	syslog(LOG_ERR, "%s(%d) nic_stats collection failed.", __FUNCTION__, __LINE__);

    return ret;
}

int get_stats(const char *ifname)
{
    memset(&ifstats, 0, sizeof(struct eth_stats));
    memset(&stat, 0, sizeof(struct ethnic_stats));

    syslog(LOG_INFO, "%s(%d): ifname is %s", __FUNCTION__, __LINE__, ifname);

    if (get_ifstats(ifname, &ifstats) < 0)
        return -1;
    if (get_ethtool_stats(ifname) < 0)
	return -1;

    return 0;
}


int linux_eth_get_stats(const char *ifname, struct eth_stats *s)
{
    if (!s)
	return -1;

    if (get_stats(ifname) < 0)
	return -1;

    s->tx_bytes = ifstats.tx_bytes;
    s->tx_packets = ifstats.tx_packets;
    s->tx_errors = ifstats.tx_errors;
    s->tx_ucast_packets = stat.data[TxUnicast];
    s->tx_mcast_packets = stat.data[TxMulticast];
    s->tx_bcast_packets = stat.data[TxBroadcast];
    s->tx_discard_packets = stat.data[TxDrop];
    s->rx_bytes = ifstats.rx_bytes;
    s->rx_packets = stat.data[rx_packets];
    s->rx_errors = ifstats.rx_errors;
    s->rx_ucast_packets = stat.data[RxUnicast];
    s->rx_mcast_packets = stat.data[RxMulticast];
    s->rx_bcast_packets = stat.data[RxBroadcast];
    s->rx_discard_packets = stat.data[RxDrop];
/*FIXME: Doesn't exist a way to get the counter rx_unknown_packets.*/
    s->rx_unknown_packets = 0;

    return 0;
}

int linux_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon)
{
    int ret = 0;

    if (get_stats(ifname) < 0)
	ret = -1;
    rmon->tx.packets = stat.data[tx_packets];
    rmon->tx.bytes = stat.data[tx_bytes];
    rmon->tx.crc_err_packets = stat.data[TxCrcErr];
/* These two counters are marked zero because they dont 
 * hold much relevancy to Customer 
 */
    rmon->tx.under_sz_packets = 0;
    rmon->tx.over_sz_packets = 0;
    rmon->tx.packets_64bytes = stat.data[TxPktSz64];
    rmon->tx.packets_65to127bytes = stat.data[TxPktSz65To127];
    rmon->tx.packets_128to255bytes = stat.data[TxPktSz128To255];
    rmon->tx.packets_256to511bytes = stat.data[TxPktSz256To511];
    rmon->tx.packets_512to1023bytes = stat.data[TxPktSz512To1023];
    rmon->tx.packets_1024to1518bytes = stat.data[Tx1024ToMax];

    rmon->rx.bytes = stat.data[RxBytes];
    rmon->rx.packets = stat.data[rx_packets];
    rmon->rx.bcast_packets = stat.data[RxBroadcast];
    rmon->rx.mcast_packets = stat.data[RxMulticast];
    rmon->rx.crc_err_packets = stat.data[RxCrcErr];
    rmon->rx.under_sz_packets = stat.data[RxUnderSizeErr];
    rmon->rx.over_sz_packets = stat.data[RxOverSzErr];
    rmon->rx.packets_64bytes = stat.data[RxPktSz64];
    rmon->rx.packets_65to127bytes = stat.data[RxPktSz65To127];
    rmon->rx.packets_128to255bytes = stat.data[RxPktSz128To255];
    rmon->rx.packets_256to511bytes = stat.data[RxPktSz256To511];
    rmon->rx.packets_512to1023bytes = stat.data[RxPktSz512To1023];
    rmon->rx.packets_1024to1518bytes = stat.data[RxPktSz1024ToMax];

    return ret;
}

const struct eth_ops linux_eth_ops = {
        .ifname = "lan",
        .get_stats = linux_eth_get_stats,
        .get_rmon_stats = linux_eth_get_rmon_stats,
};
