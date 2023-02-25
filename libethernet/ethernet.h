/*
 * ethernet.h - libethernet header file
 *
 * Copyright (C) 2020 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: anjan.chanda@iopsys.eu
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
#ifndef _ETHERNET_H
#define _ETHERNET_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/types.h>


#ifdef __cplusplus
extern "C" {
#endif

#define libethernet_err(...)	pr_error("libethernet: " __VA_ARGS__)
#define libethernet_warn(...)	pr_warn("libethernet: " __VA_ARGS__)
#define libethernet_info(...)	pr_info("libethernet: " __VA_ARGS__)
#define libethernet_dbg(...)	pr_debug("libethernet: " __VA_ARGS__)

/* enum eth_duplex - duplex modes */
enum eth_duplex {
	AUTO_DUPLEX,
	HALF_DUPLEX,
	FULL_DUPLEX,
};

/* enum eth_rmon_status - rmon status */
enum rmon_status {
	RMON_STATUS_DISABLED,
	RMON_STATUS_ENABLED,
	RMON_STATUS_ERR_MISCONF,
	RMON_STATUS_ERR,
};

/* struct eth_rmon_stats - ethernet rmon counters */
struct eth_rmon_stats {
#define ETH_TXQ_ALL	-1
	int txq;                        /* tx priority queue number or -1 */
	uint16_t vlanid;                /* 0..4094 */
	enum rmon_status status;
	struct {
		uint32_t drop_events;
		unsigned long bytes;
		unsigned long packets;
		unsigned long bcast_packets;
		unsigned long mcast_packets;
		uint32_t crc_err_packets;
		uint32_t under_sz_packets;
		uint32_t over_sz_packets;
		unsigned long packets_64bytes;
		unsigned long packets_65to127bytes;
		unsigned long packets_128to255bytes;
		unsigned long packets_256to511bytes;
		unsigned long packets_512to1023bytes;
		unsigned long packets_1024to1518bytes;
	} tx, rx;
};


#define ETH_PORT_UNDEFINED	-1

enum eth_speed {
	ETH_10_Half	= (1 << 0),
	ETH_10_Full	= (1 << 1),
	ETH_100_Half	= (1 << 2),
	ETH_100_Full	= (1 << 3),
	ETH_1000_Half	= (1 << 4),
	ETH_1000_Full	= (1 << 5),
	ETH_2500_Full	= (1 << 7),
	ETH_5000_Full	= (1 << 11),
	ETH_10000_Full	= (1 << 15),
	ETH_20000_Full	= (1 << 23),
};

struct eth_phy {
	uint8_t address;
	int portidx;
};

struct eth_info {
	int phy_portmap;
	int phy_numports;
};

enum eth_callstatus {
	ETH_SUCCESS,
	ETH_ERROR,
	ETH_EINVAL,
	ETH_EUNSUPPORTED,
	ETH_EUNKNOWN
};

struct eth_stats {
	uint64_t tx_bytes;
	uint64_t rx_bytes;
	uint64_t tx_packets;
	uint64_t rx_packets;
	uint64_t tx_errors;                 /* error packets */
	uint64_t rx_errors;
	uint64_t tx_ucast_packets;     /* unicast packets */
	uint64_t rx_ucast_packets;
	uint64_t tx_mcast_packets;     /* multicast packets */
	uint64_t rx_mcast_packets;
	uint64_t tx_bcast_packets;     /* broadcast packets */
	uint64_t rx_bcast_packets;
	uint64_t tx_discard_packets;        /* no error packets dropped */
	uint64_t rx_discard_packets;        /* no error packets dropped */
	uint64_t rx_unknown_packets;        /* unknown protocol packets */
};

/* struct eth_vlan - vlan over eth_link */
struct eth_vlan {
	ifopstatus_t status;
	uint16_t id;                       /* vlan id [1..4094] */
	uint16_t tpid;                     /* tag protocol id, t.x. 0x8100 */
	struct eth_stats stats;
};

//TODO: revisit
/* struct eth_link - logical link per ethernet port */
struct eth_link {
	int portid;
	uint32_t capability;
	bool autoneg;
	uint32_t speed;
	bool fullduplex;
	bool down;
	bool prio_tagged;
	struct eth_stats stats;
};

/* struct eth_iface - ethernet interface */
struct eth_iface {
	ifopstatus_t status;
	uint32_t lastchange;              /* in secs */
	bool is_upstream;
	uint8_t macaddr[6];
	uint32_t max_bitrate;             /* in Mbps */
	uint32_t curr_bitrate;            /* in Mbps */
	enum eth_duplex duplex;
	bool eee_supported;
	bool eee_enabled;
	struct eth_stats stats;
};

/* struct eth_device - ethernet device */
struct eth_device {
	bool wol_supported;
	uint32_t num_ifaces;
	uint32_t num_links;
	uint32_t num_vlans;
	struct eth_rmon_stats rmon;
	struct eth_iface *ifs;
	struct eth_link *links;
	struct eth_vlan *vlans;
};



#ifndef if_mii
static inline struct mii_ioctl_data *if_mii(struct ifreq *rq)
{
	return (struct mii_ioctl_data *) &rq->ifr_ifru;
}
#endif


/* TODO: eth_device/sw_ops, eth_iface_ops, eth_link_ops */

struct eth_ops {
	/** interface name/prefix to match */
	const char *ifname;

	int (*up)(const char *ifname);
	int (*down)(const char *ifname);

	int (*get_link_settings)(const char *ifname, struct eth_link *link);
	int (*set_link_settings)(const char *ifname, struct eth_link link);

	int (*get_operstate)(const char *ifname, ifopstatus_t *s);
	int (*set_operstate)(const char *ifname, ifopstatus_t s);

	int (*reset_phy)(const char *ifname, int phy_id);
	int (*poweroff_phy)(const char *ifname, struct eth_phy p);
	int (*poweron_phy)(const char *ifname, struct eth_phy p);
	int (*get_phy_id)(const char *ifname, int port, int *phy_id);

	int (*get_stats)(const char *ifname, struct eth_stats *s);
	int (*get_info)(const char *ifname, struct eth_info *info);
	int (*get_rmon_stats)(const char *ifname, struct eth_rmon_stats *rmon);
};


/* API list */
int eth_up(const char *ifname);
int eth_down(const char *ifname);
int eth_get_link_settings(const char *ifname, struct eth_link *link);
int eth_set_link_settings(const char *ifname, struct eth_link link);
int eth_get_operstate(const char *ifname, ifopstatus_t *s);
int eth_set_operstate(const char *ifname, ifopstatus_t s);
int eth_get_stats(const char *ifname, struct eth_stats *c);
int eth_get_info(const char *ifname, struct eth_info *info);
int eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon);

int eth_reset_phy(const char *ifname, int phy_id);
int eth_poweroff_phy(const char *ifname, struct eth_phy p);
int eth_poweron_phy(const char *ifname, struct eth_phy p);

int eth_get_phy_id(const char *ifname, int port, int *phy_id);

int eth_ioctl(const char *ifname, int cmd, void *in, int len);
int eth_mii_read(const char *ifname, int phy_id, int reg, int *out);
int eth_mii_write(const char *ifname, int phy_id, int reg, int in);
int eth_mii_get_phy_id(const char *ifname, int port, int *phy_id);
int eth_mii_reset_phy(const char *ifname, int phy_id);

#ifdef __cplusplus
}
#endif
#endif /* _ETHERNET_H */
