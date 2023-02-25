#ifndef LIBEASY_QOS_H
#define LIBEASY_QOS_H

/* Needed to get well-sized integers */
#include <stdint.h>

/* Needed to get the IFNAMSIZ define */
#include <net/if.h>

/* For Error logging */
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IOPSYS_BROADCOM
extern const struct qos_ops bcm_rdpa_ops;
#endif
#ifdef IOPSYS_TEST
extern const struct qos_ops qos_test_ops;
#endif
#ifdef IOPSYS_ECONET
extern const struct qos_ops qos_ecnt_ops_eth;
extern const struct qos_ops qos_ecnt_ops_nas;
extern const struct qos_ops qos_ecnt_ops_ae_wan;
#endif
#if defined(IOPSYS_LINUX) || defined(IPQ95XX)
extern const struct qos_ops qos_linux_ops_eth;
#endif

/* TBD: are these structures generic enough to support any QoS stats?
   So far they are derived from the Broadcom queue stats structures, but in the
   future Mediatek networking chipset support would be added too. */

/** Scheduling algorithms possible for a queue */
enum qos_scheduler {
	scheduler_invalid = 0,
	SP,
	WRR,
	SP_WRR,
	WFQ
};

/** This struct represents the QoS statistics of a queue. */
struct qos_stats {
    uint64_t tx_packets;
    uint64_t tx_bytes;
    uint64_t tx_dropped_packets;
    uint64_t tx_dropped_bytes;
};

/** Used to configure a queue. */
struct qos_queue_config {
	char iface[IFNAMSIZ];
	enum qos_scheduler scheduler;
	int32_t precedence;
	int32_t rate;
	int32_t burst_size;
	int32_t weight;
};

struct qos_ops {
	/** interface name/prefix to match */
	const char *ifname;

	int (*get_stats)(const char *ifname, int queue_id, struct qos_stats *stats, int *is_read_and_reset);
};

/* API list */
int qos_get_stats(const char *ifname, int queue_id, struct qos_stats *stats, int *is_read_and_reset);

#ifdef __cplusplus
}
#endif

#endif // LIBEASY_QOS_H
