/*
 * bcm_qos_runner.c - Broadcom RDPA QoS library implementation
 *
 * Copyright (C) 2021 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: amit.kumar@iopsys.eu
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include <bcmnet.h>
#include <bcmswapitypes.h>
#include <rdpa_types.h>
#include <rdpa_drv.h>
#include <rdpa_port.h>
#include <rdpa_port_user_ioctl_ag.h>
#include <rdpa_egress_tm_user_ioctl_ag.h>
#include <rdpa_types.h>
#include <rdpa_user.h>
#include <rdpa_egress_tm.h>
#include <board.h>

#include "qos.h"

#define IFNAME_SIZE 16
#define RDPACTL_IOCTL_FILE_NAME "/dev/bcmrdpa"
#define RDPA_USR_DEV_NAME "/dev/rdpa_user"

#define get_dir(dev_type, rdpa_if) \
  ((((dev_type == RDPA_IOCTL_DEV_PORT) && (!rdpa_if_is_wan(rdpa_if))) ||\
    (dev_type == RDPA_IOCTL_DEV_NONE))? rdpa_dir_ds : rdpa_dir_us)

static int rdpa_egress_tm_queue_stat_get(bdmf_object_handle tm_obj, bdmf_index idx, rdpa_stat_1way_t * queue_stat);
static int get_queue_tm(bdmf_object_handle root_tm, int qid, bdmf_object_handle *egress_tm, int *idx);
static int get_root_tm(bdmf_object_handle owner, rdpa_port_tm_cfg_t *tm_cfg);
static int get_tm_owner(char *ifname, bdmf_object_handle *owner);

/**
 *  Initialize the socket to bcmsw interface
 *  @param p_ifr output parameter pointer to ifreq structure
 *  return : integer +ve socket file discriptor on success and -1 for failure.
 */
static inline int qos_socket_init(struct ifreq *p_ifr)
{
	int skfd;
	char wan[IFNAME_SIZE] = {0};

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethswctl */
	strncpy(p_ifr->ifr_name, "bcmsw", strlen("bcmsw"));
	p_ifr->ifr_name[strlen("bcmsw")] = '\0';
	if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
		// As per bcmkernel eth0 seems to be reserved for bcmsw failsafe
		strncpy(p_ifr->ifr_name, "eth0", strlen("eth0"));
		p_ifr->ifr_name[strlen("eth0")] = '\0';
		if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
			close(skfd);
			syslog(LOG_ERR, "neither bcmsw nor eth0 exist\n");
			return -1;
		}
	}

	return skfd;
}

/**
 *  To get rdpa interface form linux interface name like eth0 etc.
 *  @param ifname input parameter pointer to linux interface name
 *  @param rdpa_if_p output parameter pointer to rdpa interface
 *  return : integer +ve socket file discriptor on success and -1 for failure.
 */
static int bcm_enet_get_rdpa_if_from_ifname(const char* ifname, int* rdpa_if_p)
{
	int skfd, err = 0;
	struct ifreq ifr;
	struct ethswctl_data ifdata = {0};
	struct ethswctl_data *e = &ifdata;

	if ((skfd = qos_socket_init(&ifr)) < 0) {
		syslog(LOG_ERR, "qos_socket_init failed. \n");
		return skfd;
	}
	ifr.ifr_data = (char *)&ifdata;

	e->op = ETHSWRDPAPORTGETFROMNAME;
	e->type = TYPE_GET;
	strncpy(e->ifname, ifname, strlen(ifname));

	if((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
		syslog(LOG_ERR, "ioctl command return error %d\n", err);
		close(skfd);
		return err;
	} else {
		syslog(LOG_INFO, "ifname - %s rdpa_if %d\n", e->ifname, e->val);
		close(skfd);
		*rdpa_if_p = e->val;
		return err;
	}
}

/**
 *  Actual ioctl call for rdpa file /dev/bcmrdpa
 *  @param code input parameter to RDPA_IOC_TM
 *  @param ctx output parameter for ioctl
 *  return : integer 0 on success and -1 on failure.
 */
static inline int rdpa_ioctl(int code, uintptr_t ctx)
{
	int fd, ret = 0;
	fd = open(RDPACTL_IOCTL_FILE_NAME, O_RDWR);
	if (fd < 0) {
		syslog(LOG_ERR, "Open system call failed for : %s : %d\n", RDPACTL_IOCTL_FILE_NAME, fd);
		return -1;
	}

	ret = ioctl(fd, code, ctx);
	if (ret)
		syslog(LOG_ERR, "ioctl failed, ret=%d\n", ret);

	close(fd);
	return ret;
}

/**
 * Fill rdpa structure and call ioctl to get stat
 *  @param dev_type input parameter for device type
 *  @param dev_id input parameter for device id
 *  @param dir input parameter for traffic direction
 *  @param qid input parameter for queue id
 *  @param queue_tat output parameter pointer to rdpa_stat_1way_t
 *  return : integer 0 on success and -1 on failure.
 */
static int rdpa_ctl_get_queue_stats(int dev_type, char *ifname, int qid, rdpa_stat_1way_t *queue_stat)
{
	bdmf_object_handle owner;
	rdpa_port_tm_cfg_t tm_cfg;
	bdmf_object_handle root_tm;
	bdmf_object_handle egress_tm;
	int ret = -1;
	int idx = 0;

	ret = get_tm_owner(ifname, &owner);
	if (ret != 0) {
		syslog(LOG_ERR, "error in reading tm_owner\n");
		return ret;
	}
	ret = get_root_tm(owner, &tm_cfg);
	if (ret != 0) {
		syslog(LOG_ERR, "error in getting root_tm\n");
		return ret;
	}
	root_tm = tm_cfg.sched;
	ret = get_queue_tm(root_tm, qid, &egress_tm, &idx);
	if (ret != 0) {
		syslog(LOG_ERR, "error in getting queue_tm\n");
		return ret;
	}
	ret = rdpa_egress_tm_queue_stat_get(egress_tm, idx, queue_stat);
	if (ret != 0) {
		syslog(LOG_ERR, "error in rdpa_egress_tm_queue_stat_get\n");
		return ret;
	}

	syslog(LOG_INFO, "[Stats] PASSED: packets=%u bytes=%u  DISCARDED: packets=%u bytes=%u\n",
	       queue_stat->passed.packets, queue_stat->passed.bytes,
	       queue_stat->discarded.packets, queue_stat->discarded.bytes);

	return ret;
}

/**
 *  Final queue stats to be filled in qos_stat structure from rdpa_get_queue_stats
 *  @param dev_type input parameter for device type
 *  @param rdpa_if input parameter for actual rdpa interface
 *  @param qid input parameter for queue id
 *  @param qstats output parameter pointer to qos_stats
 *  return : integer 0 on success and -1 on failure.
 */
static int get_bcm_queue_stats(int dev_type, char *ifname, int qid,
			      struct qos_stats *qstats)
{
	int ret = 0;

	int  rc;
	rdpa_stat_1way_t stats;

	if ((rc = rdpa_ctl_get_queue_stats(dev_type, ifname, qid, &stats))) {
		syslog(LOG_ERR, "rdpa_ctl_get_queue_stats ERROR! ifname=%s qid=%d rc=%d\n",
		       ifname, qid, rc);
		return -1;
	}

	qstats->tx_packets = stats.passed.packets;
	qstats->tx_bytes = stats.passed.bytes;
	qstats->tx_dropped_packets = stats.discarded.packets;
	qstats->tx_dropped_bytes = stats.discarded.bytes;

	return ret;
}

/**
 * read the CHIP id from board database.
 * BCM968 CHIP, stats read from driver is accumulated stats, while in other its read and reset
 **/
static int get_is_driver_stats_read_and_reset()
{
	BOARD_IOCTL_PARMS IoctlParms;
	int fd;
	char result_chip_id[20];
	int len = 0;


	memset(result_chip_id, '\0', 20);
	fd = open("/dev/brcmboard", O_RDWR);
	if (fd == -1) {
		return 0;
	}

	IoctlParms.string = NULL;
	IoctlParms.strLen = 0;
	IoctlParms.offset = 0;
	IoctlParms.action = 0;
	IoctlParms.buf    = "";
	if ( ioctl(fd, BOARD_IOCTL_GET_CHIP_ID, &IoctlParms) < 0 ) {
		close(fd);
		return 0;
	}
	close(fd);
	len = sprintf(&result_chip_id[0], "%X", IoctlParms.result);
	result_chip_id[len] = '\0';
	syslog(LOG_INFO,"%s result_chip_id:%s\n",__FUNCTION__,result_chip_id);
	/*BCM968 CHIP, stats read from driver is accumulated stats, while in other its read and reset */
	if ((strncmp(result_chip_id, "68", 2) == 0) || (strncmp(result_chip_id, "675", 3) == 0) ||
			(strncmp(result_chip_id, "6315",4) == 0))
		return 0;
	else
		return 1;

}

/**
 *  To get the queue stats for an RDPA device:
   - determine the RDPA device ID through the bcmsw device
   - Fill up a rdpa_drv_ioctl_tm_t struct with the request and the RDPA
     device ID, pass it through an ioctl to /dev/bcmrdpa
 *  @param ifname input parameter for linux interface
 *  @param qid input parameter for queue id
 *  @param qstats output parameter pointer to qos_stats
 *  @param is_read_and_reset output parameter for stat fetch was read and reset for driver
 */
static int bcm_get_stats(const char *ifname,
			 int queue_id, struct qos_stats *stats, int *is_read_and_reset)
{
	int ret;

	ret = get_bcm_queue_stats(0, ifname, queue_id, stats);
	if (ret == -1) {
		syslog(LOG_ERR,"Cannot find stats for ifname:%s\n", ifname);
		return ret;
	}
	*is_read_and_reset = get_is_driver_stats_read_and_reset();

	return ret;
}

const struct qos_ops bcm_rdpa_ops = {
	.ifname = "eth",
	.get_stats = bcm_get_stats,
};

/**
 *  Fetch tm owner(port reference) from driver
 *  @param if_id rdpa interface id
 *  @param owner reference to tm owner port inde in driver
 *  return : integer 0 on success and -1 on failure.
 **/
static int get_tm_owner(char *ifname, bdmf_object_handle *owner)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd;
	int ret = -1;

	pa.cmd = RDPA_PORT_GET;
	pa.ptr = (bdmf_ptr)(unsigned long)ifname;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0) {
		syslog(LOG_ERR, "%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -1;
	}
	ret = ioctl(fd, RDPA_PORT_IOCTL, &pa);
	if (ret) {
		syslog(LOG_ERR, "ioctl failed, Errno[%s] ret=%d\n", strerror(errno), ret);
		close(fd);
		return ret;
	}

	*owner = pa.mo;
	close(fd);

	return ret;
}


/**
 *  Fetch reference to root of tm for interface
 *  @param owner reference to qos port index fetched from driver
 *  @param tm_cfg reference to root of tm config for interface
 *  return : integer 0 on success and -1 on failure.
 **/
static int get_root_tm(bdmf_object_handle owner, rdpa_port_tm_cfg_t *tm_cfg)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = owner;
	pa.ptr = (bdmf_ptr)(unsigned long)tm_cfg;
	pa.cmd = RDPA_PORT_TM_CFG_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0) {
		return -1;
	}
	ret = ioctl(fd, RDPA_PORT_IOCTL, &pa);
	if (ret) {
		close(fd);
	return ret;
	}

	close(fd);
	return pa.ret;
}

/**
 *  Fetch reference to egress tm for qos queue from driver
 *  @param root_tm reference to root of tm config for interface
 *  @param qid qos queue id
 *  @param egress_tm reference to root of tm config for interface
 *  return : integer 0 on success and -1 on failure.
 **/
static int get_queue_tm(bdmf_object_handle root_tm, int qid, bdmf_object_handle *egress_tm, int *idx)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;
	rdpa_tm_queue_location_t  queue_location;

	pa.mo = root_tm;
	pa.ai = (bdmf_index)(long)qid;
	pa.ptr = (bdmf_ptr)(unsigned long)&queue_location;
	pa.cmd = RDPA_EGRESS_TM_QUEUE_LOCATION_GET;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0) {
		return -1;
	}
	ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
	if (ret) {
	close(fd);
		return ret;
	}

	*idx = queue_location.queue_idx;
	*egress_tm = queue_location.queue_tm;
	close(fd);
	return pa.ret;
}

/**
 *  Final queue stats to be fethed from rdpa driver
 *  @param tm_obj refernce for qos queue index from driver
 *  @param qidx input parameter for queue id
 *  @param queue_stat output parameter pointer to qos_stats
 *  return : integer 0 on success and -1 on failure.
 **/
static int rdpa_egress_tm_queue_stat_get(bdmf_object_handle tm_obj, bdmf_index idx, rdpa_stat_1way_t * queue_stat)
{
        rdpa_ioctl_cmd_t pa = {0};
        int fd, ret;

        pa.mo = tm_obj;
        pa.ai = (bdmf_index)(long)idx;
        pa.ptr = (bdmf_ptr)(unsigned long)queue_stat;
        pa.cmd = RDPA_EGRESS_TM_QUEUE_STAT_GET;

        fd = open(RDPA_USR_DEV_NAME, O_RDWR);
        if (fd < 0) {
                return -1;
        }
        ret = ioctl(fd, RDPA_EGRESS_TM_IOCTL, &pa);
        if (ret) {
                close(fd);
                return ret;
        }

        close(fd);
        return pa.ret;
}

