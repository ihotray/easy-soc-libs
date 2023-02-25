/*
 * ethsw.c - implements APIs through swlib
 *
 * Copyright (C) 2018 iopsys Software Solutions AB. All rights reserved.
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
//#include <net/if.h>
#include <linux/if.h>
#include <stdbool.h>
#include <linux/mii.h>

#include <linux/netlink.h>
#include <linux/switch.h>
#include <swlib.h>

#include "ethernet.h"

#define UNUSED(var)	(void)(var)

#define ETHSW_NAME	"switch0"

int ethsw_get_link_settings(char *name, struct eth_link *link)
{
	struct switch_dev *sw_dev;
	struct switch_attr *attr;
	struct switch_val val;
	struct switch_port_link *l;
	int port = ETH_PORT_UNDEFINED;

	if (link->portid != ETH_PORT_UNDEFINED)
		port = link->portid;

	//eth_get_port_data(name, PORT_TYPE_NUMBER); // TODO ?
	if (port == ETH_PORT_UNDEFINED)
		return -1;

	sw_dev = swlib_connect(ETHSW_NAME);
	if (!sw_dev)
		return -1;

	swlib_scan(sw_dev);
        val.port_vlan = port;
	attr = swlib_lookup_attr(sw_dev, SWLIB_ATTR_GROUP_PORT, "link");
	if (attr->type != SWITCH_TYPE_LINK){
		swlib_free_all(sw_dev);
		return -1;
	}
	swlib_get_attr(sw_dev, attr, &val);
	l = val.value.link;
	if (l->link){
		link->autoneg = l->aneg;
		link->fullduplex = l->duplex;
		link->speed = l->speed;
		link->down = 0;
	} else {
		link->autoneg = 0;
		link->fullduplex = 0;
		link->speed = 0;
		link->down = 1;
	}
	swlib_free_all(sw_dev);

	return 0;
}

int ethsw_set_link_settings(char *name, struct eth_link link)
{
	struct switch_dev *sw_dev;
	struct switch_attr *attr;
	struct switch_val val;
	struct switch_port_link l;
	int port = ETH_PORT_UNDEFINED;

	libethernet_dbg("%s(): name = %s\n", __func__, name);
	if (link.portid != ETH_PORT_UNDEFINED)
		port = link.portid;

	//eth_get_port_data(name, PORT_TYPE_NUMBER); // TODO ?
	if (port == ETH_PORT_UNDEFINED)
		return -1;

	sw_dev = swlib_connect(ETHSW_NAME);
	if (!sw_dev)
		return -1;

	swlib_scan(sw_dev);
        val.port_vlan = port;
	attr = swlib_lookup_attr(sw_dev, SWLIB_ATTR_GROUP_PORT, "link");
	if (attr->type != SWITCH_TYPE_LINK){
		swlib_free_all(sw_dev);
		return -1;
	}

	memset(&l, 0, sizeof(struct switch_port_link));
	l.duplex = link.fullduplex;
	l.aneg = link.autoneg;
	l.speed = link.speed;
	val.value.link = &l;
	swlib_set_attr(sw_dev, attr, &val);
	swlib_free_all(sw_dev);

	return 0;
}


int ethsw_poweron_phy(char *name, struct eth_phy p)
{
	return 0;
}

int ethsw_poweroff_phy(char *name, struct eth_phy p)
{
	return 0;
}

const struct eth_ops ethsw_ops = {
	.name = "eth",
	//.up = ethsw_up,
	//.down = ethsw_down,
	.set_link_settings = ethsw_set_link_settings,
	.get_link_settings = ethsw_get_link_settings,
	//.get_stats = ethsw_get_stats,
	.poweron_phy = ethsw_poweron_phy,
	.poweroff_phy = ethsw_poweroff_phy,
};
