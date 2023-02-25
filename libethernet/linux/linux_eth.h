#ifndef LINUX_ETH_H
#define LINUX_ETH_H

#ifdef __cplusplus
extern "C" {
#endif

int linux_eth_get_stats(const char *ifname, struct eth_stats *s);
int linux_eth_get_rmon_stats(const char *ifname, struct eth_rmon_stats *rmon);

#ifdef __cplusplus
}
#endif

#endif /* LINUX_ETH_H */
