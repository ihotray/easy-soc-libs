#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/netlink.h>
#include <linux/pkt_sched.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/msg.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/netlink.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/class.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc/cbq.h>
#include <netlink/route/addr.h>
#include <netlink/cli/utils.h>

#include "../include/qos.h"

#include "easy.h"

#define MAXQES 8

static struct qos_stats queue_stats[MAXQES];

struct queue_desc {
    unsigned ifindex;
    char ifname[IFNAMSIZ];
    int qid;
    struct qos_stats *qstats;
} q_desc[MAXQES];

struct glbl_cntxt {
    int quid;
    int count;
    int ifindex;
    struct nl_sock *sock;
    struct rtnl_link *if_lnk;
    struct nl_cache *qdisc_cache;
    struct nl_cache *class_cache;
} g;

static void in_qdisc(struct nl_object *, void *);
static void all_tc_children(struct rtnl_tc *);

/*********************************************
 * Statistics collector from tc .
 * The stats counters on corresponding structs.
 * are populated for all queues at once.
 *********************************************
 */
static  void get_stat(struct nl_object *obj)
{
    struct rtnl_tc *tc = nl_object_priv(obj);
    struct qos_stats *q = q_desc[g.count].qstats;

    if ((int)g.ifindex != rtnl_tc_get_ifindex(tc)) {
        syslog(LOG_ERR, "ifindex did not match. q_desc.idx is %d"
                  " rtnl idx is %d", g.ifindex, rtnl_tc_get_ifindex(tc));
        return;
    }
    q->tx_packets = rtnl_tc_get_stat(tc, RTNL_TC_PACKETS);
    q->tx_bytes = rtnl_tc_get_stat(tc, RTNL_TC_BYTES);
    q->tx_dropped_packets = rtnl_tc_get_stat(tc, RTNL_TC_DROPS);
    g.count++;
    g.count &= (MAXQES-1);
}

struct rtnl_class *nl_class_alloc(void)
{
    struct rtnl_class *class;

    if (!(class = rtnl_class_alloc()))
        syslog(LOG_ERR, "Unable to allocate class object");

    return class;
}

/***************************************************
 * Traverse the tree rooted at this class.
 ***************************************************
 */
static void in_class(struct nl_object *obj, void *arg)
{
    struct rtnl_qdisc *leaf;
    struct rtnl_class *class = (struct rtnl_class *) obj;
    struct nl_cache *cls_cache;
    uint32_t parent = rtnl_tc_get_handle((struct rtnl_tc *) class);

    leaf = rtnl_class_leaf_qdisc(class, g.qdisc_cache);
    if (leaf) {
        in_qdisc((struct nl_object *)leaf, &(g.count));
    }

    all_tc_children(TC_CAST(class));
    if (rtnl_cls_alloc_cache(g.sock, g.ifindex, parent, &cls_cache) < 0) {
        syslog(LOG_ERR, "Unable to allocate nl object");
        return;
    }
    get_stat((struct nl_object *)class);
    nl_cache_free(cls_cache);
}

/****************************************************
 * Iterate on all the classes and find the class
 * corresponding to the queue id supplied to us.
 ****************************************************
 */
static void all_tc_children(struct rtnl_tc *tc)
{
    struct rtnl_class *filter;

    filter = nl_class_alloc();
    if (!filter) {
        syslog(LOG_ERR, "Unable to allocate nl object");
        return;
    }
	
    rtnl_tc_set_parent(TC_CAST(filter), rtnl_tc_get_handle(tc));
    rtnl_tc_set_ifindex(TC_CAST(filter), rtnl_tc_get_ifindex(tc));

    nl_cache_foreach_filter(g.class_cache, OBJ_CAST(filter), &in_class, NULL);

    rtnl_class_put(filter);
}

/*******************************************************
 * Qdisc parser.
 *******************************************************
 */
static void in_qdisc(struct nl_object *obj, void *n)
{
    if (n) {
        get_stat(obj);
    }
    struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) obj;
    all_tc_children(TC_CAST(qdisc));
}

/**************************************************************
 * Entry point in to tc parsing state machine.
 * At this point we have the tree rooted at TC_H_ROOT("root").
 * Will parse down this tree until we received the queue stats.
 **************************************************************
 */
static void iterate_On_link_from_ifname(struct nl_object *obj)
{
    struct rtnl_qdisc *qdisc;

    if (!obj)
	return;

    if (rtnl_class_alloc_cache(g.sock, g.ifindex, &(g.class_cache)) < 0) {
        syslog(LOG_ERR, "Unable to allocate nl object");
	return;
    }

    qdisc = rtnl_qdisc_get_by_parent(g.qdisc_cache, g.ifindex, TC_H_ROOT);
    if (qdisc) {
        get_stat((struct nl_object *)qdisc);
        in_qdisc((struct nl_object *) qdisc, NULL);
        rtnl_qdisc_put(qdisc);
    }
    qdisc = rtnl_qdisc_get_by_parent(g.qdisc_cache, g.ifindex, 0);
    if (qdisc) {
        in_qdisc((struct nl_object *) qdisc, NULL);
        rtnl_qdisc_put(qdisc);
    }
    nl_cache_free(g.class_cache);
}

/********************************************************
 * Open, connect  NL socket.
 ********************************************************
 */
static int open_nl_socket(void)
{
    int ret = 0;

    g.sock = nl_socket_alloc();
    if (g.sock == NULL) {
        ret = -errno;
        return ret;
    }
    if ((ret = nl_connect(g.sock, NETLINK_ROUTE)) < 0) {
        syslog(LOG_ERR, "Unable to connect netlink socket.");
        ret = abs(ret);
        goto out;
    }
    return 0;

out:
    nl_socket_free(g.sock);
    return ret;
}


static int close_link_iface(struct nl_sock *s)
{
    nl_socket_free(s);

    return 0;
}

static struct nl_cache *nl_allocate_qdisc_cache(struct nl_sock *s,
	const char *name, int (*ac)(struct nl_sock *, struct nl_cache **))
{
    struct nl_cache *cache;
    int err;

    if ((err = ac(s, &cache)) < 0)
        syslog(LOG_ERR, "Unable to allocate  cache");

    nl_cache_mngt_provide(cache);

    return cache;
}

static inline struct nl_cache *alloc_cache_family_flags(void)
{
    struct nl_cache *cache;
    int err;

    if ((err = rtnl_link_alloc_cache_flags(g.sock, AF_UNSPEC,
                    &cache, 0)) < 0) {
        syslog(LOG_ERR, "Unable to allocate link cache");
    }
    nl_cache_mngt_provide(cache);
    return cache;
}

/*******************************************************
 * Get corresponding link cache for interface.
 *******************************************************
 */
static void get_link_cache(struct nl_object *obj, void *arg)
{
    struct rtnl_link *link = (struct rtnl_link *) obj;

    if (link) {
        if (g.ifindex == rtnl_link_get_ifindex(link)) {
	    g.if_lnk = link;
        }
    }
}
	
/*******************************************************
 *  Get the queue stats from tc stats using NL socket. 
 *  @param ifname: input parameter for linux interface
 *  @param qid: input parameter for queue id
 *******************************************************
 */
int  qos_get_queue_stats(const char* intname, int q_id)
{
    struct nl_cache *link_cache = NULL;
    int ret = 0;
    ret = open_nl_socket();
    if (ret) {
        return ret;
    }
    link_cache = alloc_cache_family_flags();
    g.qdisc_cache = nl_allocate_qdisc_cache(g.sock, "queueing disciplines",
		   rtnl_qdisc_alloc_cache);
    if (!(g.qdisc_cache)) {
        ret = -1;
        goto out;
    }
    nl_cache_foreach(link_cache, &get_link_cache, NULL);
    if (!g.if_lnk) {
        syslog(LOG_ERR, "%s(%d):if_lnk is NULL ", __FUNCTION__, __LINE__);
        ret = -1;
        goto out;
    }
    iterate_On_link_from_ifname((struct nl_object *)g.if_lnk);

out:
    if (g.qdisc_cache)
        nl_cache_free(g.qdisc_cache);
    if (link_cache)
        nl_cache_free(link_cache);
    (void)close_link_iface(g.sock);
    return ret;
}


static void describe_queue(int q_id, char *intname,
                struct qos_stats *stats)
{
    int i;

    for (i = 0; i < MAXQES ; i++) {
        strcpy(q_desc[i].ifname, intname);
        q_desc[i].ifindex = (unsigned int)g.ifindex;
        q_desc[i].qid = i;
        q_desc[i].qstats = &queue_stats[i];
        q_desc[i].qstats->tx_packets = 0;
        q_desc[i].qstats->tx_bytes = 0;
        q_desc[i].qstats->tx_dropped_packets = 0;
    }
}

/*******************************************************
 *  To get the queue stats for a linux device:
 *  @param ifname: input parameter for linux interface
 *  @param qid: input parameter for queue id
 *  @param qstats: output parameter pointer to qos_stats
 *  @param is_read_and_reseti: output parameter for stat
 *  fetch was read and reset for driver.
 *******************************************************
 */
static int linux_get_stats(const char *ifname,
                int queue_id, struct qos_stats *stats,
                int *is_read_and_reset)
{
    int ret = 0;
    char iface[IFNAMSIZ];

    strcpy(iface, ifname);
    g.ifindex = (int)if_nametoindex(ifname);
    g.count = 0;
    g.quid = queue_id;
    g.sock = NULL;
    g.if_lnk = NULL;
    g.qdisc_cache = NULL;
    g.class_cache = NULL;
    describe_queue(queue_id, iface, stats);
    ret = qos_get_queue_stats(ifname, queue_id);

    stats->tx_packets = q_desc[queue_id].qstats->tx_packets;
    stats->tx_bytes = q_desc[queue_id].qstats->tx_bytes;
    stats->tx_dropped_packets = q_desc[queue_id].qstats->tx_dropped_packets;
    *is_read_and_reset = 0;
    return ret;
}

#if defined(IOPSYS_LINUX)
const struct qos_ops qos_linux_ops_eth = {
    .ifname = "lan",
    .get_stats = linux_get_stats,
};
#elif defined(IPQ95XX)
const struct qos_ops qos_linux_ops_eth = {
    .ifname = "eth",
    .get_stats = linux_get_stats,
};
#endif
