/*
 * econet.c - implements libethernet functions for Econet switch
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
#include "ecnt_prvt.h"

int econet_eth_set_link_settings(const char *name, struct eth_link link)
{
	UNUSED(name);
	UNUSED(link);
	libethernet_err("%s(): TODO\n", __func__);
	return 0;
}

int econet_eth_get_link_settings(const char *ifname, struct eth_link *link)
{
	uint32_t port_num;

	port_num = ecnt_prvt_get_port_num(ifname);
	if (port_num == ECNT_PRVT_PORT_NUM_INVALID) {
		libethernet_err("invalid port name: %s\n", ifname);
		return -1;
	}

	return ecnt_prvt_get_link_settings(port_num, link);
}

int econet_eth_get_stats(const char *ifname, struct eth_stats *stats)
{
	uint32_t port_num;

	port_num = ecnt_prvt_get_port_num(ifname);
	if (port_num == ECNT_PRVT_PORT_NUM_INVALID) {
		libethernet_err("invalid port name: %s\n", ifname);
		return -1;
	}

	if (ecnt_prvt_get_port_statistics(port_num, stats, NULL)) {
		libethernet_err("error reading stats for interface %s\n", ifname);
		return -1;
	}

	return 0;
}

int econet_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rstats)
{
	uint32_t port_num;

	port_num = ecnt_prvt_get_port_num(ifname);
	if (port_num == ECNT_PRVT_PORT_NUM_INVALID) {
		libethernet_err("invalid port name: %s\n", ifname);
		return -1;
	}

	if (ecnt_prvt_get_port_statistics(port_num, NULL, rstats)) {
		libethernet_err("error reading rmon stats for interface %s\n", ifname);
		return -1;
	}

	return 0;
}

int econet_eth_poweron_phy(const char *ifname, struct eth_phy p)
{
	uint32_t port_num;

	port_num = ecnt_prvt_get_port_num(ifname);
	if (port_num == ECNT_PRVT_PORT_NUM_INVALID) {
		libethernet_err("invalid port name: %s\n", ifname);
		return -1;
	}

	return ecnt_prvt_set_port_state(port_num, true);
}

int econet_eth_poweroff_phy(const char *ifname, struct eth_phy p)
{
	uint32_t port_num;

	port_num = ecnt_prvt_get_port_num(ifname);
	if (port_num == ECNT_PRVT_PORT_NUM_INVALID) {
		libethernet_err("invalid port name: %s\n", ifname);
		return -1;
	}

	return ecnt_prvt_set_port_state(port_num, false);
}

int econet_eth_reset_phy(const char *name, int phy_id)
{
	UNUSED(name);
	UNUSED(phy_id);
	libethernet_err("%s(): TODO\n", __func__);
	return 0;
}

/* Declare separate eth ops for all known Econet interfaces */
#define ECNT_ETH_OPS(__name, __interface_name)          \
const struct eth_ops __name = {                         \
	.ifname = (__interface_name),                       \
	.set_link_settings = econet_eth_set_link_settings,  \
	.get_link_settings = econet_eth_get_link_settings,  \
	.get_stats = econet_eth_get_stats,                  \
	.get_rmon_stats = econet_eth_get_rmon_stats,        \
	.poweron_phy = econet_eth_poweron_phy,              \
	.poweroff_phy = econet_eth_poweroff_phy,            \
	.reset_phy = econet_eth_reset_phy,                  \
}

ECNT_ETH_OPS(econet_gen_eth_ops, "eth");
ECNT_ETH_OPS(econet_nas_wan_eth_ops, "nas");
ECNT_ETH_OPS(econet_ae_wan_eth_ops, "ae_wan");

#undef ECNT_ETH_OPS
