/*
 * bcm.c - implements for Broadcom eth switch
 *
 * Copyright (C) 2018 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: anjan.chanda@iopsys.eu
 *	   oussama.ghorbel@iopsys.eu
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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <stdbool.h>
#include <linux/mii.h>

#include "easy.h"
#include "../ethernet.h"
#include "bcmswapitypes.h"
#include "bcmnet.h"

/* invalid stat counter values */
#define INVALID_UINT64	UINT64_MAX
#define INVALID_UINT32	UINT32_MAX
#define INVALID_UINT16	UINT16_MAX
#define INVALID_UINT8	UINT8_MAX


#define PHYID_2_MII_IOCTL(phy_id, mii_ioctl_data) \
                    {mii_ioctl_data->val_out = (phy_id >> 24 ) & 0xff; \
                    mii_ioctl_data->phy_id = phy_id & 0x1f;}

static int bcm_eth_get_unit_port(const char *ifname, int *unit, int *port)
{
	struct ethswctl_data data;
	int ret;

	memset(&data, 0, sizeof(struct ethswctl_data));

	data.op = ETHSWUNITPORT;
	data.type = TYPE_GET;
	strncpy(data.ifname, ifname, 16);

	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed! ret = %d\n", ret);
		return -1;
	}

	*unit = data.unit;
	*port = data.port;

	if (!data.port && data.port_map) {
		int i;
		unsigned int portmap = data.port_map;

		for (i = 0; i < 8 * sizeof(portmap); i++) {
			if (!!(portmap & (1UL << i))) {
				*port = i;
				break;
			}
		}
	}

	/* libethernet_dbg("[%s] unit = %d port = %d portmap = 0x%x "
			"phyportmap = 0x%x\n",
			ifname, *unit, *port, data.port_map,
			data.phy_portmap); */

	return 0;
}

int bcm_eth_get_link_settings(const char *ifname, struct eth_link *link)
{
	struct ethswctl_data data;
	int unit = -1;
	int port = -1;
	int ret;

	memset(&data, 0, sizeof(struct ethswctl_data));
	ret = bcm_eth_get_unit_port(ifname, &unit, &port);
	if (ret)
		return -1;

	data.op = ETHSWPHYMODE;
	data.port = port;
	data.unit = unit;
	data.type = TYPE_GET;

	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed!\n");
		return -1;
	}

	link->portid = port;
	link->speed = data.speed;
	link->fullduplex = data.duplex == 1 ? false : true;

	/* libethernet_dbg("port: %d  speed = %d  fullduplex = %d\n",
			link->portid, link->speed, link->fullduplex); */

	if (!!(data.phycfg & PHY_CFG_1000FD))
		link->capability |= ETH_1000_Full;

	if (!!(data.phycfg & PHY_CFG_1000HD))
		link->capability |= ETH_1000_Half;

	if (!!(data.phycfg & PHY_CFG_100FD))
		link->capability |= ETH_100_Full;

	if (!!(data.phycfg & PHY_CFG_100HD))
		link->capability |= ETH_100_Half;

	if (!!(data.phycfg & PHY_CFG_10FD))
		link->capability |= ETH_10_Full;

	if (!!(data.phycfg & PHY_CFG_10HD))
		link->capability |= ETH_10_Half;

	if (!!(data.phycfg & PHY_CFG_5000FD))
		link->capability |= ETH_5000_Full;

	if (!!(data.phycfg & PHY_CFG_10000FD))
		link->capability |= ETH_10000_Full;


	memset(&data, 0, sizeof(struct ethswctl_data));
	data.op = ETHSWPHYAUTONEG;
	data.port = port;
	data.unit = unit;
	data.type = TYPE_GET;

	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed! ret = %d\n", ret);
		return -1;
	}

	link->autoneg = data.autoneg_info == 0 ? false : true;
	/* libethernet_dbg("autoneg = %d\n", link->autoneg); */

	memset(&data, 0, sizeof(struct ethswctl_data));
	data.op = ETHSWLINKSTATUS;
	data.port = port;
	data.unit = unit;
	data.type = TYPE_GET;

	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed!\n");
		return -1;
	}

	link->down = data.status == 0 ? true : false;

	return 0;
}

int bcm_eth_set_link_settings(const char *name, struct eth_link link)
{
	libethernet_err("%s(): TODO\n", __func__);
	return 0;
}

int bcm_eth_poweron_phy(const char *name, struct eth_phy p)
{
	struct ethctl_data data;

	memset(&data, 0, sizeof(struct ethctl_data));
	data.op = ETHSETPHYPWRON;

	if (eth_ioctl(name, SIOCETHSWCTLOPS, &data,
				sizeof(struct ethswctl_data)) < 0)
		return -1;

	return 0;
}

int bcm_eth_poweroff_phy(const char *name, struct eth_phy p)
{
	struct ethctl_data data;

	memset(&data, 0, sizeof(struct ethctl_data));
	data.op = ETHSETPHYPWROFF;

	if (eth_ioctl(name, SIOCETHSWCTLOPS, &data,
				sizeof(struct ethswctl_data)) < 0)
		return -1;

	return 0;
}

int bcm_eth_reset_phy(const char *name, int phy_id)
{
	return eth_mii_reset_phy(name, phy_id & 0x1f);
}

static int bcm_eth_get_stats_from_proc(const char *ifname, struct eth_stats *s)
{
	uint64_t rx_bytes, rx_packets, rx_err, rx_drop;
	uint64_t rx_fifo, rx_frame, rx_comp, rx_multi;
	uint64_t tx_bytes, tx_packets, tx_err, tx_drop;
	uint64_t tx_fifo, tx_coll, tx_carr, tx_comp;
	uint64_t tx_mcast_packets = 0, rx_mcast_bytes = 0, tx_mcast_bytes = 0;
	uint64_t rx_ucast_packets = 0, tx_ucast_packets = 0;
	uint64_t rx_bcast_packets = 0, tx_bcast_packets = 0;
	uint64_t rx_err_unknown = 0;
	char cmdbuf[512] = {0};
	char *ptr;
	int ret;

	chrCmd(cmdbuf, sizeof(cmdbuf),
		"cat /proc/net/dev_extstats | grep %s:",
		ifname);

	if (cmdbuf[0] == '\0' || strncmp(cmdbuf, ifname, strlen(ifname)))
		return -1;

	ptr = cmdbuf + strlen(ifname) + strlen(":");
	ret = sscanf(ptr,
		" %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64
		" %" SCNu64 " %" SCNu64
		" %" SCNu64,
		&rx_bytes, &rx_packets, &rx_err, &rx_drop,
		&rx_fifo, &rx_frame, &rx_comp, &rx_multi,
		&tx_bytes, &tx_packets, &tx_err, &tx_drop,
		&tx_fifo, &tx_coll, &tx_carr, &tx_comp,
		&tx_mcast_packets, &rx_mcast_bytes, &tx_mcast_bytes,
		&rx_ucast_packets, &tx_ucast_packets,
		&rx_bcast_packets, &tx_bcast_packets,
		&rx_err_unknown);

	if (ret < 16)
		return -1;

	s->tx_bytes = tx_bytes;
	s->rx_bytes = rx_bytes;
	s->tx_packets = tx_packets;
	s->rx_packets = rx_packets;
	s->tx_errors = tx_err;
	s->rx_errors = rx_err;
	s->tx_discard_packets = tx_drop;
	s->rx_discard_packets = rx_drop;
	s->tx_ucast_packets = tx_ucast_packets;
	s->rx_ucast_packets = rx_ucast_packets;
	s->tx_mcast_packets = tx_mcast_packets;
	s->rx_mcast_packets = 0;	/* 'rx_mcast_bytes' is only available */
	s->tx_bcast_packets = tx_bcast_packets;
	s->rx_bcast_packets = rx_bcast_packets;
	s->rx_unknown_packets = rx_err_unknown;

	return 0;
}


static int read_eth_stats(const char *ifname, struct eth_stats *s)
{
	int ret_proc;
	struct ethswctl_data data;
	int unit = -1;
	int port = -1;
	int ret;

	/* get from proc first */
	ret_proc = bcm_eth_get_stats_from_proc(ifname, s);
	if (!ret_proc)
		return 0;

	memset(&data, 0, sizeof(struct ethswctl_data));
	ret = bcm_eth_get_unit_port(ifname, &unit, &port);
	if (ret)
		return -1;

	data.op = ETHSWEMACGET;
	data.port = port;
	data.unit = unit;
	data.queue = -1;	/* all */

	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed! ret = %d\n", ret);
		return -1;
	}

	s->tx_bytes = data.emac_stats_s.tx_byte;
	s->rx_bytes = data.emac_stats_s.rx_byte;
	s->tx_packets = data.emac_stats_s.tx_packet;
	s->rx_packets = data.emac_stats_s.rx_packet;
	s->tx_errors = data.emac_stats_s.tx_error;
	s->rx_errors = data.emac_stats_s.rx_fcs_error +
			data.emac_stats_s.rx_alignment_error +
			data.emac_stats_s.rx_frame_length_error +
			data.emac_stats_s.rx_code_error +
			data.emac_stats_s.rx_carrier_sense_error +
			data.emac_stats_s.rx_undersize_packet +
			data.emac_stats_s.rx_oversize_packet;


	s->tx_ucast_packets = data.emac_stats_s.tx_unicast_packet;
	s->rx_ucast_packets = data.emac_stats_s.rx_unicast_packet;
	s->tx_mcast_packets = data.emac_stats_s.tx_multicast_packet;
	s->rx_mcast_packets = data.emac_stats_s.rx_multicast_packet;
	s->tx_bcast_packets = data.emac_stats_s.tx_broadcast_packet;
	s->rx_bcast_packets = data.emac_stats_s.rx_broadcast_packet;
	s->rx_unknown_packets = data.emac_stats_s.rx_unknown_opcode;

	return 0;
}


static void reinit_extended_stats_for_bridge(struct eth_stats *to)
{
	// In case of bridge, function to read all stats has already
	// been called once, which might have stored garbage in the
	// extended stats, so important to reinit here.
	to->tx_ucast_packets = 0;
	to->rx_ucast_packets = 0;
	to->tx_mcast_packets = 0;
	to->rx_mcast_packets = 0;
	to->tx_bcast_packets = 0;
	to->rx_bcast_packets = 0;
	to->rx_unknown_packets = 0;
}


// This function adds up the extended stats to get cumulative stats
// for the bridge type interfaces.
static void add_to_stats(struct eth_stats *to, struct eth_stats *from)
{
	to->tx_ucast_packets += from->tx_ucast_packets;
	to->rx_ucast_packets += from->rx_ucast_packets;
	to->tx_mcast_packets += from->tx_mcast_packets;
	to->rx_mcast_packets += from->rx_mcast_packets;
	to->tx_bcast_packets += from->tx_bcast_packets;
	to->rx_bcast_packets += from->rx_bcast_packets;
	to->rx_unknown_packets += from->rx_unknown_packets;
}


int bcm_eth_get_stats(const char *ifname, struct eth_stats *s)
{

	if (read_eth_stats(ifname, s)) {
		libethernet_err("error in reading stats for interface %s\n", ifname);
		return -1;
	}

	if (if_isbridge(ifname)) {
		// read stats for each interface in bridge and add them to get
		// bridge stats
		struct eth_stats temp;
		int ret = 0;
		char ifnames[32][16] = {0};
		int count, i;

		ret = br_get_iflist(ifname, &count, ifnames);
		if ((count <= 0) || (ret < 0)) // empty bridge
			return 0;

		// only extended stats are not available for bridge interfaces,
		// we have already read the available stats above and now loop
		// on the member ports to get the extended stats for each port
		// and add those up to get the extended stats for the bridge
		reinit_extended_stats_for_bridge(s);
		for (i = 0; i < count; i++) {
			memset(&temp, 0, sizeof(struct eth_stats));

			if (strncmp("eth", ifnames[i], 3) != 0)
				continue;

			if (read_eth_stats(ifnames[i], &temp)) {
				libethernet_err("error in reading stats for interface %s\n", ifnames[i]);
				continue; // no need to add this to bridge stats
			}

			add_to_stats(s, &temp);
		}
	}

	return 0;
}

int bcm_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon)
{
	struct ethswctl_data data;
	int unit = -1;
	int port = -1;
	int ret;

	if (!rmon)
		return -1;

	memset(&data, 0, sizeof(struct ethswctl_data));
	ret = bcm_eth_get_unit_port(ifname, &unit, &port);
	if (ret)
		return -1;

	//data.op = ETHSWDUMPMIB;
	data.op = ETHSWEMACGET;
	data.port = port;
	data.unit = unit;
	data.queue = rmon->txq < 0 ? -1 : rmon->txq;


	ret = eth_ioctl(ifname, SIOCETHSWCTLOPS, &data,
						sizeof(struct ethswctl_data));
	if (ret != 0) {
		libethernet_err("ioctl failed! ret = %d\n", ret);
		return -1;
	}

	rmon->tx.bytes = data.emac_stats_s.tx_byte;
	rmon->tx.packets = data.emac_stats_s.tx_packet;
	rmon->tx.bcast_packets = data.emac_stats_s.tx_broadcast_packet;
	rmon->tx.mcast_packets = data.emac_stats_s.tx_multicast_packet;
	rmon->tx.crc_err_packets = data.emac_stats_s.tx_fcs_error;
	rmon->tx.under_sz_packets = data.emac_stats_s.tx_undersize_frame;
	rmon->tx.over_sz_packets = data.emac_stats_s.tx_oversize_frame;
	rmon->tx.packets_64bytes = data.emac_stats_s.tx_frame_64;
	rmon->tx.packets_65to127bytes = data.emac_stats_s.tx_frame_65_127;
	rmon->tx.packets_128to255bytes = data.emac_stats_s.tx_frame_128_255;
	rmon->tx.packets_256to511bytes = data.emac_stats_s.tx_frame_256_511;
	rmon->tx.packets_512to1023bytes = data.emac_stats_s.tx_frame_512_1023;
	rmon->tx.packets_1024to1518bytes = data.emac_stats_s.tx_frame_1024_1518;

	rmon->rx.bytes = data.emac_stats_s.rx_byte;
	rmon->rx.packets = data.emac_stats_s.rx_packet;
	rmon->rx.bcast_packets = data.emac_stats_s.rx_broadcast_packet;
	rmon->rx.mcast_packets = data.emac_stats_s.rx_multicast_packet;
	rmon->rx.crc_err_packets = data.emac_stats_s.rx_fcs_error;
	rmon->rx.under_sz_packets = data.emac_stats_s.rx_undersize_packet;
	rmon->rx.over_sz_packets = data.emac_stats_s.rx_oversize_packet;
	rmon->rx.packets_64bytes = data.emac_stats_s.rx_frame_64;
	rmon->rx.packets_65to127bytes = data.emac_stats_s.rx_frame_65_127;
	rmon->rx.packets_128to255bytes = data.emac_stats_s.rx_frame_128_255;
	rmon->rx.packets_256to511bytes = data.emac_stats_s.rx_frame_256_511;
	rmon->rx.packets_512to1023bytes = data.emac_stats_s.rx_frame_512_1023;
	rmon->rx.packets_1024to1518bytes = data.emac_stats_s.rx_frame_1024_1518;

	return 0;
}

const struct eth_ops bcm_eth_ops = {
	.ifname = "eth",
	//.up = bcm_eth_up,
	//.down = bcm_eth_down,
	.set_link_settings = bcm_eth_set_link_settings,
	.get_link_settings = bcm_eth_get_link_settings,
	.get_stats = bcm_eth_get_stats,
	.get_rmon_stats = bcm_eth_get_rmon_stats,
	.poweron_phy = bcm_eth_poweron_phy,
	.poweroff_phy = bcm_eth_poweroff_phy,
	.reset_phy = bcm_eth_reset_phy,
};
