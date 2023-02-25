#ifndef IPQ95XX
#define IPQ95XX

#ifdef __cplusplus
extern "C" {
#endif

int ipq95xx_eth_set_link_settings(const char *ifname, struct eth_link link);
int ipq95xx_eth_get_link_settings(const char *ifname, struct eth_link *link);
int ipq95xx_eth_poweron_phy(const char *ifname, struct eth_phy p);
int ipq95xx_eth_poweroff_phy(const char *ifname, struct eth_phy p);
int ipq95xx_eth_reset_phy(const char *ifname, int phy_id);
int ipq95xx_eth_get_stats(const char *ifname, struct eth_stats *s);
int ipq95xx_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon);

#ifdef __cplusplus
}
#endif

#endif /* IPQ95XX */
