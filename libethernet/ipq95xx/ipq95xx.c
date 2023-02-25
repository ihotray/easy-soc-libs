/*
 * ipq95xx.c
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
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include "easy.h"
#include "if_utils.h"
#include "../ethernet.h"

#include "ipq95xx.h"

int ipq95xx_eth_set_link_settings(const char *ifname, struct eth_link link)
{
	struct {
		struct ethtool_link_settings req;
		__u32 link_mode_data[3 * 32 * 127];
	} ecmd;

	memset(&ecmd, 0, sizeof(struct ethtool_link_settings));

	if (ipq95xx_eth_get_link_settings(ifname, &link))
		return -1;

	ecmd.req.cmd = ETHTOOL_SLINKSETTINGS;

	ecmd.req.port = link.portid;
	ecmd.req.speed = link.speed;
	ecmd.req.autoneg = (link.autoneg == 0 ? false : true);
	ecmd.req.duplex = (link.fullduplex == 1 ? false : true);

	if (0 != eth_ioctl(ifname, SIOCETHTOOL, &ecmd, sizeof(struct ethtool_link_settings)))
		return -1;
	return 0;
}

int ipq95xx_eth_get_link_settings(const char *ifname, struct eth_link *link)
{
	struct {
		struct ethtool_link_settings req;
		__u32 link_mode_data[3 * 32 * 127];
	} ecmd;

	memset(&ecmd, 0, sizeof(struct ethtool_link_settings));

	ecmd.req.cmd = ETHTOOL_GLINKSETTINGS;

	if (0 != eth_ioctl(ifname, SIOCETHTOOL, &ecmd, sizeof(struct ethtool_link_settings)))
		return -1;

	if (ecmd.req.link_mode_masks_nwords >= 0 || ecmd.req.cmd != ETHTOOL_GLINKSETTINGS)
		return -1;

	ecmd.req.link_mode_masks_nwords = -ecmd.req.link_mode_masks_nwords;

	if (0 != eth_ioctl(ifname, SIOCETHTOOL, &ecmd, sizeof(struct ethtool_link_settings)))
		return -1;

	if (ecmd.req.link_mode_masks_nwords <= 0 || ecmd.req.cmd != ETHTOOL_GLINKSETTINGS)
		return -1;

	link->portid = ecmd.req.port;
	link->speed = ecmd.req.speed;
	link->autoneg = ecmd.req.autoneg == 0 ? false : true;
	link->fullduplex = ecmd.req.duplex == 1 ? false : true;

	return 0;
}

int ipq95xx_eth_poweron_phy(const char *ifname, struct eth_phy p)
{
	libethernet_err("%s(): TODO\n", __func__);
	return 0;
}

int ipq95xx_eth_poweroff_phy(const char *ifname, struct eth_phy p)
{
	libethernet_err("%s(): TODO\n", __func__);
	return 0;
}

int ipq95xx_eth_reset_phy(const char *ifname, int phy_id)
{
	return eth_mii_reset_phy(ifname, phy_id);
}

int ipq95xx_eth_get_stats(const char *ifname, struct eth_stats *s)
{
	struct if_stats ifstats;

	memset(&ifstats, 0, sizeof(struct if_stats));

	if (if_getstats(ifname, &ifstats) < 0)
		return -1;

	s->rx_packets = ifstats.rx_packets;
	s->tx_packets = ifstats.tx_packets;
	s->rx_bytes = ifstats.rx_bytes;
	s->tx_bytes = ifstats.tx_bytes;
	s->rx_errors = ifstats.rx_errors;
	s->tx_errors = ifstats.tx_errors;
	s->rx_discard_packets = ifstats.rx_dropped;
	s->tx_discard_packets = ifstats.tx_dropped;

	return 0;
}

int ipq95xx_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon)
{
	libethernet_err("%s(): TODO\n", __func__);
        return 0;
}

const struct eth_ops ipq95xx_eth_ops = {
        .ifname = "eth",
        .set_link_settings = ipq95xx_eth_set_link_settings,
        .get_link_settings = ipq95xx_eth_get_link_settings,
        .get_stats = ipq95xx_eth_get_stats,
        .get_rmon_stats = ipq95xx_eth_get_rmon_stats,
        .poweron_phy = ipq95xx_eth_poweron_phy,
        .poweroff_phy = ipq95xx_eth_poweroff_phy,
        .reset_phy = ipq95xx_eth_reset_phy,
};
