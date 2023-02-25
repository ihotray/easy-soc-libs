/*
 * ecnt_prvt.c - Econet switch private utilities
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
#include <ctype.h>
#include <fcntl.h>
#include <linux/mii.h>
#include <net/if.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "easy.h"
#include "../ethernet.h"

#include "ecnt_prvt.h"

#ifdef STATIC_ANALYSIS
#include "stub_libapi_lib_switchmgr.h"
#else
#include "libapi_lib_switchmgr.h"
#endif

#define TC3162_MAX_LINE_LEN     (100)
#define TC3162_DUPLEX_MODE_LEN  (32)
#define TC3162_SPEED_MODE_LEN   (32)
#define TC3162_ETH_PORTMAP_PATH "/proc/tc3162/eth_portmap"
#define IFNAME_ETH0             "eth0."
#define IFNAME_NAS              "nas"
#define IFNAME_AE_WAN           "ae_wan"

/* Not defined in Econet library */
ECNT_SWITCHMGR_RET switchmgr_lib_get_port_link_state(u8 port,
	ECNT_SWITCHMGR_LINK_STATE *p_link_state,
	ECNT_SWITCHMGR_LINK_SPEED *p_speed);

int ecnt_prvt_get_port_statistics(uint32_t port,
								  struct eth_stats *stats,
								  struct eth_rmon_stats *rstats)
{
	int ret;
	ECNT_SWITCHMGR_PORT_STATISTICS portcnt;

	if (port == ECNT_PRVT_PORT_NUM_INVALID ||
		(stats == NULL && rstats == NULL)) {
		return -1;
	}

	if (port >= 256) {
		return -1;
	}

	memset(&portcnt, 0, sizeof(portcnt));

	ret = switchmgr_lib_get_port_statistics((uint8_t)port, &portcnt);
	if (ret != ECNT_SWITCHMGR_SUCCESS) {
		return -1;
	}

	if (stats != NULL) {
		stats->tx_bytes =
			portcnt.TxBytesCnt_Lo;
		stats->rx_bytes =
			portcnt.RxBytesCnt_Lo;
		stats->tx_packets = portcnt.TxPktsCnt;
		stats->rx_packets = portcnt.RxPktsCnt;
		stats->tx_errors = 0;
		stats->rx_errors = 0;
		stats->tx_ucast_packets = portcnt.TxUniPktsCnt;
		stats->rx_ucast_packets = portcnt.RxUniPktsCnt;
		stats->tx_mcast_packets = portcnt.TxMultiPktsCnt;
		stats->rx_mcast_packets = portcnt.RxMultiPktsCnt;
		stats->tx_bcast_packets = portcnt.TxBroadPktsCnt;
		stats->rx_bcast_packets = portcnt.RxBroadPktsCnt;
		stats->tx_discard_packets = portcnt.TxDropFramesCnt;
		stats->rx_discard_packets = portcnt.RxDropFramesCnt;
		stats->rx_unknown_packets = 0;
	}

	if (rstats != NULL) {
#define FILL_RSTATS_FOR_DIRECTION(__rmon_field, __ecnt_prefix) \
		do { \
			rstats->tx.drop_events = \
				portcnt.__ecnt_prefix ## DropFramesCnt; \
			rstats->__rmon_field.bytes = \
				portcnt.__ecnt_prefix ## BytesCnt_Lo; \
			rstats->__rmon_field.packets = portcnt.__ecnt_prefix ## PktsCnt;\
			rstats->__rmon_field.bcast_packets = \
				portcnt.__ecnt_prefix ## BroadPktsCnt; \
			rstats->__rmon_field.mcast_packets = \
				portcnt.__ecnt_prefix ## MultiPktsCnt; \
			rstats->__rmon_field.crc_err_packets = \
				portcnt.__ecnt_prefix ## CRCFramesCnt; \
			rstats->__rmon_field.under_sz_packets = \
				portcnt.__ecnt_prefix ## UnderSizePktsCnt; \
			rstats->__rmon_field.over_sz_packets = \
				portcnt.__ecnt_prefix ## OverSizePktsCnt; \
			rstats->__rmon_field.packets_64bytes = \
				portcnt.__ecnt_prefix ## 64BytePktsCnt; \
			rstats->__rmon_field.packets_65to127bytes = \
				portcnt.__ecnt_prefix ## 65_127BytePktsCnt; \
			rstats->__rmon_field.packets_256to511bytes = \
				portcnt.__ecnt_prefix ## 256_511BytePktsCnt; \
			rstats->__rmon_field.packets_512to1023bytes = \
				portcnt.__ecnt_prefix ## 512_1023BytePktsCnt; \
			rstats->__rmon_field.packets_1024to1518bytes = \
				portcnt.__ecnt_prefix ## 1024_1518BytePktsCnt; \
		} while (0)

		FILL_RSTATS_FOR_DIRECTION(tx, Tx);
		FILL_RSTATS_FOR_DIRECTION(rx, Rx);

#undef FILL_RSTATS_FOR_DIRECTION
	}

	return 0;
}

/* Read port number from proc interface */
static uint32_t read_port_from_proc(bool wan, uint32_t eth_port_num)
{
	FILE    *f;
	int      tmp;
	int      ret;
	char     buf[TC3162_MAX_LINE_LEN];
	uint32_t new_port_num = ECNT_PRVT_PORT_NUM_INVALID;

	f = fopen(TC3162_ETH_PORTMAP_PATH, "r");
	if (f == NULL) {
		return ECNT_PRVT_PORT_NUM_INVALID;
	}

	if (wan) {
		/* WAN is the very first in the proc dump */
		ret = fscanf(f, "%d", &tmp);
		if (ret != 1) {
			goto out;
		}

		new_port_num = (uint32_t)tmp;
	} else {
		char   *retstr;
		bool    inside_map = false;

		/* First we search for the beginning of switch port map */
		while (true) {
			retstr = fgets(buf, sizeof(buf), f);
			if (retstr == NULL) {
				break;
			}

			inside_map = strstr(buf, "switch_port_map") != NULL;
			if (inside_map) {
				break;
			}
		}

		if (!inside_map) {
			goto out;
		}

		/*
		 * Then we sequentially read all ports until the needed port line is
		 * found
		 * Line format: <virtual port> <switch port>
		 */
		while (true) {
			int port;
			int swport;

			if (fscanf(f, "%d %d", &port, &swport) != 2) {
				break;
			}

			if (port == eth_port_num) {
				new_port_num = (uint32_t)swport;
				goto out;
			}
		}
	}

out:
	fclose(f);

	return new_port_num;
}

uint32_t ecnt_prvt_get_port_num(const char *ifname)
{
	bool     is_wan;
	uint32_t eth_port_num = 0;
	/*
	 * This algorithm is not accurate if numbers differ for target hardware.
	 * There must be a generic way to get port IDs reliably based on interface
	 * names.
	 */
	if (strncmp(ifname, IFNAME_ETH0, strlen(IFNAME_ETH0)) == 0) {
		char *end;

		is_wan = false;
		eth_port_num = (uint32_t)strtol(&ifname[strlen(IFNAME_ETH0)], &end, 10);

		if (end == NULL || *end != '\0') {
			return ECNT_PRVT_PORT_NUM_INVALID;
		}
	} else if (strncmp(ifname, IFNAME_NAS, strlen(IFNAME_NAS)) == 0 ||
			   strncmp(ifname, IFNAME_AE_WAN, strlen(IFNAME_AE_WAN)) == 0) {
		is_wan = true;
	} else {
		return ECNT_PRVT_PORT_NUM_INVALID;
	}

	return read_port_from_proc(is_wan, eth_port_num);
}


enum eth_speed
ecnt_prvt_link_speed2lib(int speed)
{
	switch (speed)
	{
#define MAP_ECNT_LINK_SPEED(__val, __speed, __duplex) \
		case (__val): \
		{ \
			return (__speed); \
		}
#include "map_link_speed.def"
#undef MAP_ECNT_LINK_SPEED

		default:
			return ETH_10_Half;
	}
}

uint32_t
ecnt_prvt_bitrate2capability(char *speed_str, char *duplex_mode_str)
{
	int   speed;
	bool  full_duplex;
	char *end;

	full_duplex = strcmp(duplex_mode_str, "Full") == 0;

	if (strcmp(speed_str, "Auto") == 0) {
		goto err;
	}

	speed = strtol(speed_str, &end, 10);
	if (end == NULL || *end != '\0') {
		goto err;
	}

	/* Convert bitrate and duplex mode to actual capability */
#define MAP_ECNT_BITRATE_RANGE_BOUNDARY(__bound, __full_cap, __half_cap) \
	do { \
		if (speed >= (__bound)) { \
			if (full_duplex) { \
				return __full_cap; \
			} else { \
				return __half_cap; \
			} \
		} \
	} while (0);
#include "map_bitrate.def"
#undef MAP_ECNT_BITRATE_RANGE_BOUNDARY

err:
	/* Fail-safe defaults */
	return full_duplex ? ETH_100_Full : ETH_100_Half;
}

int ecnt_prvt_get_link_settings(uint32_t port_num, struct eth_link *link)
{
	uint8_t  up;
	uint8_t  autoneg;
	char     duplex_mode[TC3162_DUPLEX_MODE_LEN];
	char     speed_str[TC3162_SPEED_MODE_LEN];
	ECNT_SWITCHMGR_LINK_STATE link_state;
	ECNT_SWITCHMGR_LINK_SPEED link_speed;

	if (port_num >= 256) {
		return -1;
	}

	if (switchmgr_lib_get_port_admin((uint8_t)port_num, &up) !=
			ECNT_SWITCHMGR_SUCCESS ||
		switchmgr_lib_get_port_autoneg_enable((uint8_t)port_num, &autoneg) !=
			ECNT_SWITCHMGR_SUCCESS ||
		switchmgr_lib_get_port_link_state((uint8_t)port_num, &link_state, &link_speed) !=
			ECNT_SWITCHMGR_SUCCESS ||
		switchmgr_lib_get_port_duplex((uint8_t)port_num, duplex_mode) !=
			ECNT_SWITCHMGR_SUCCESS ||
		switchmgr_lib_get_port_max_bitrate((uint8_t)port_num, speed_str) !=
			ECNT_SWITCHMGR_SUCCESS) {
		return -1;
	}

	link->portid = (int)port_num;
	link->capability = ecnt_prvt_bitrate2capability(speed_str, duplex_mode);
	link->autoneg = autoneg;
	link->speed = ecnt_prvt_link_speed2lib(link_speed);
	link->fullduplex = (strcmp(duplex_mode, "Full") == 0) ? true : false;
	link->down = (link_state == ECNT_SWITCHMGR_LINK_UP) ? false : true;
	link->prio_tagged = false;

	return 0;
}

int ecnt_prvt_set_port_state(uint32_t port_num, bool state)
{
	int ret;

	if (port_num >= 256) {
		return -1;
	}

	ret = switchmgr_lib_set_port_admin((uint8_t)port_num, state ? 1 : 0);

	return (ret == ECNT_SWITCHMGR_SUCCESS) ? 0 : (-1);
}

