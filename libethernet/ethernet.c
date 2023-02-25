/*
 * ethernet.c - file implements library APIs
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
#include <stdbool.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <net/if.h>
#include "easy.h"

#include "ethernet.h"


#if defined(IOPSYS_BROADCOM)
extern const struct eth_ops bcm_eth_ops;
#elif defined(IOPSYS_TEST)
extern const struct eth_ops test_eth_ops;
#elif defined(IOPSYS_ECONET)
extern const struct eth_ops econet_gen_eth_ops;
extern const struct eth_ops econet_nas_wan_eth_ops;
extern const struct eth_ops econet_ae_wan_eth_ops;
#elif defined(IPQ95XX)
extern const struct eth_ops ipq95xx_eth_ops;
#elif defined(IOPSYS_LINUX)
extern const struct eth_ops linux_eth_ops;
#else
extern const struct eth_ops ethsw_ops;
#endif

const struct eth_ops *eth_ops[] = {
#if defined(IOPSYS_BROADCOM)
	&bcm_eth_ops,
#elif defined(IOPSYS_TEST)
	&test_eth_ops,
#elif defined(IOPSYS_ECONET)
	&econet_gen_eth_ops,
	&econet_nas_wan_eth_ops,
	&econet_ae_wan_eth_ops,
#elif defined(IPQ95XX)
	&ipq95xx_eth_ops,
#elif defined(IOPSYS_LINUX)
        &linux_eth_ops,
#else
	&ethsw_ops,   /* FIXME */
#endif
};

const struct eth_ops *get_eth_driver(const char *ifname)
{
	int i;

	for (i = 0; i < sizeof(eth_ops)/sizeof(eth_ops[0]); i++) {
		if (!strncmp(eth_ops[i]->ifname, ifname,
					strlen(eth_ops[i]->ifname)))
			return eth_ops[i];
	}

	// The plan is to have this support in broadcom only for now, that is,
	// we will present bridge level stats for broadcom only for now, hence,
	// this, in the long term, we should add this support for all platforms.
#ifdef IOPSYS_BROADCOM
	if (if_isbridge(ifname)) {
		return eth_ops[0];
	}
#endif
	return NULL;
}

int eth_up(const char *ifname)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->up)
		return eth->up(ifname);

	return -1;
}

int eth_down(const char *ifname)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->down)
		return eth->down(ifname);

	return -1;
}

int eth_get_link_settings(const char *ifname, struct eth_link *link)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->get_link_settings)
		return eth->get_link_settings(ifname, link);

	return -1;
}

int eth_set_link_settings(const char *ifname, struct eth_link link)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->set_link_settings)
		return eth->set_link_settings(ifname, link);

	return -1;
}

int eth_get_operstate(const char *ifname, ifopstatus_t *s)
{
	return get_ifoperstatus(ifname, s);
}

int eth_set_operstate(const char *ifname, ifopstatus_t s)
{
	/* return set_ifoperstatus(ifname, s); */	//TODO
	return -1;
}

int eth_get_stats(const char *ifname, struct eth_stats *s)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->get_stats)
		return eth->get_stats(ifname, s);

	return -1;
}

int eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->get_rmon_stats)
		return eth->get_rmon_stats(ifname, rmon);

	return -1;
}

int eth_poweron_phy(const char *ifname, struct eth_phy p)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->poweron_phy)
		return eth->poweron_phy(ifname, p);

	return -1;
}

int eth_poweroff_phy(const char *ifname, struct eth_phy p)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->poweroff_phy)
		return eth->poweroff_phy(ifname, p);

	return -1;
}

int eth_reset_phy(const char *ifname, int phy_id)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->reset_phy)
		return eth->reset_phy(ifname, phy_id);
	else
		return eth_mii_reset_phy(ifname, phy_id);
}

int eth_get_phy_id(const char *ifname, int port, int *phy_id)
{
	const struct eth_ops *eth = get_eth_driver(ifname);

	if (eth && eth->get_phy_id)
		return eth->get_phy_id(ifname, port, phy_id);
	else
		return eth_mii_get_phy_id(ifname, port, phy_id);
}

int eth_ioctl(const char *ifname, int cmd, void *in, int len)
{
	struct ifreq ifr;
	int s;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		libethernet_err("SIOCGIFINDEX failed!\n");
		close(s);
		return -1;
	}

	if (len && in)
		ifr.ifr_data = (caddr_t)in;

	if (ioctl(s, cmd, &ifr) < 0) {
		close(s);
		return -1;
	}

	close(s);
	return 0;
}

int eth_mii_get_phy_id(const char *ifname, int port, int *phy_id)
{
	struct mii_ioctl_data *mii;
	struct ifreq ifr;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		libethernet_err("SIOCGIFINDEX failed!\n");
		close(s);
		return -1;
	}

	mii = if_mii(&ifr);
	memset(mii, 0, sizeof(struct mii_ioctl_data));
	mii->val_in = (uint16_t)port;

	if (ioctl(s, SIOCGMIIPHY, &ifr) < 0) {
		libethernet_err("SIOCGMIIPHY failed!\n");
		close(s);
		return -1;
	}

	if (phy_id)
		*phy_id = mii->phy_id;

	//libethernet_dbg("%s: phy_id = %d\n", ifname, *phy_id);
	close(s);
	return 0;
}

static int eth_mii_ioctl(const char *ifname, int cmd, int phy_id, int reg,
						int in, int *out)
{
	struct mii_ioctl_data *mii;
	struct ifreq ifr;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		libethernet_err("SIOCGIFINDEX failed!\n");
		close(s);
		return -1;
	}

	mii = if_mii(&ifr);
	memset(mii, 0, sizeof(struct mii_ioctl_data));

	//PHYID_2_MII_IOCTL(phy_id, mii);
	mii->phy_id = (uint16_t)phy_id;
	mii->reg_num = (uint16_t)reg;
	mii->val_in = (uint16_t)in;

	if (ioctl(s, cmd, &ifr) < 0) {
		libethernet_err("MII cmd on %s failed\n", ifr.ifr_name);
		close(s);
		return -1;
	}

	if (out)
		*out = mii->val_out;	// FIXME?

	close(s);
	return 0;
}

int eth_mii_read(const char *ifname, int phy_id, int reg, int *out)
{
	return eth_mii_ioctl(ifname, SIOCGMIIREG, phy_id, reg, 0, out);
}

int eth_mii_write(const char *ifname, int phy_id, int reg, int in)
{
	return eth_mii_ioctl(ifname, SIOCGMIIREG, phy_id, reg, in, NULL);
}

int eth_mii_reset_phy(const char *ifname, int phy_id)
{
	return eth_mii_ioctl(ifname, SIOCSMIIREG, phy_id,
				MII_BMCR, BMCR_RESET, NULL);
}
