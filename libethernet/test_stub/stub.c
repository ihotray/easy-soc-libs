/*
 * stub.c - implements 'test' ethernet
 *
 * Copyright (C) 2021 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: jomily.joseph@iopsys.eu
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
#include <sys/types.h>
#include <fcntl.h>
#include <net/if.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>

#include "easy.h"
#include "ethernet.h"

#include "stub.h"

int test_eth_set_link_settings(const char *name, struct eth_link link)
{
	return 0;
}

int test_eth_get_link_settings(const char *name, struct eth_link link)
{
	return 0;
}

int test_eth_poweron_phy(const char *name, struct eth_phy p)
{
	return 0;
}

int test_eth_poweroff_phy(const char *name, struct eth_phy p)
{
	return 0;
}

int test_eth_reset_phy(const char *name, int phy_id)
{
	return 0;
}

static int test_eth_get_stats_from_proc(const char *ifname, struct eth_stats *s) 
{
        GET_TEST_STATS(s, ifname, eth_stats, struct eth_stats);
	return 0;
}

int test_eth_get_stats(const char *ifname, struct eth_stats *s)
{
	test_eth_get_stats_from_proc(ifname, s);
        GET_TEST_STATS(s, ifname, eth_stats, struct eth_stats);

        return 0;
}

int test_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon)
{
        GET_TEST_STATS(rmon, ifname, eth_rmon_stats, struct eth_rmon_stats);

        return 0;
}

const struct eth_ops test_eth_ops = {
        .ifname = "eth",
        .set_link_settings = test_eth_set_link_settings,
        .get_link_settings = test_eth_get_link_settings,
        .get_stats = test_eth_get_stats,
        .get_rmon_stats = test_eth_get_rmon_stats,
        .poweron_phy = test_eth_poweron_phy,
        .poweroff_phy = test_eth_poweroff_phy,
        .reset_phy = test_eth_reset_phy,
};
