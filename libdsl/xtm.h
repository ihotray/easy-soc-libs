/*
 * xtm.h - library header file
 * This file provides definition for the libdsl APIs and related
 * structures.
 *
 * Copyright (C) 2020 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: yalu.zhang@iopsys.eu
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
#ifndef _XTM_H_
#define _XTM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** enum atm_link_type - The type of the connection and the complete stack of protocol used for this connection */
enum atm_link_type {
	ATM_LINK_UNCONFIGURED = 0,
	ATM_LINK_EoA,   /** Bridged Ethernet over ATM */
	ATM_LINK_IPoA,  /** Routed IP over ATM */
	ATM_LINK_PPPoA, /** PPP over ATM */
	ATM_LINK_CIP    /** Classical IP over ATM */
};

/** struct atm_dest_addr - Destination address of this link, i.e. a pair of VPI and VCI */
struct atm_dest_addr {
	unsigned int vpi;
	unsigned int vci;
};

/** enum atm_encapsulation - ATM connection encapsulation */
enum atm_encapsulation {
	ATM_LLC = 1, /** Multiple protocols are transmitted over the same ATM VC (Virtual Connection) */
	ATM_VCMUX    /** Each protocol is transmitted over a separate ATM VC */
};

/** enum atm_aal - AAL (ATM Adaptation Layer) currently used on the PVC */
enum atm_aal {
	ATM_AAL1 = 1,
	ATM_AAL2,
	ATM_AAL3,
	ATM_AAL4,
	ATM_AAL5
};

/** struct atm_link - ATM link configuration and status */
struct atm_link {
	/** RO The current operational status of the link */
	enum itf_status status;
	/** RW Link type */
	enum atm_link_type link_type;
	/** RO Indicates that some auto configuration mechanism is used for this connection currently */
	bool auto_config;
	/** RW Destination address */
	struct atm_dest_addr dest_addr;
	/** RW Encapsulation */
	enum atm_encapsulation encapsulation;
	/** RW Whether checksum should be added in the ATM payload. It only applies to the upstream direction */
	bool fcs_preserved;

	/** RW Search list of destination addresses if the configured VPI/VCI pair cannot be established */
	struct atm_dest_addr *vc_search_list;
	unsigned int vc_list_count;

	/** RO Current ALL in use */
	enum atm_aal aal;
};

/**
 * struct atm_link_stats - Statistics of an ATM link
 * Note that only ATM specific statistics counters are defined since generic statistics are define
 * in an IP interface
 */
struct atm_link_stats {
	/** Successfully transmitted cells */
	unsigned int transmitted_blocks;
	/** Successfully received cells */
	unsigned int received_blocks;
	/** Count of ATM layer CRC errors */
	unsigned int crc_errors;
};

/** enum atm_qos_class - ATM QoS used on the VC */
enum atm_qos_class {
	ATM_QoS_UBR = 1,
	ATM_QoS_CBR,
	ATM_QoS_GFR,
	ATM_QoS_VBR_nrt,
	ATM_QoS_VBR_rt,
	ATM_QoS_UBR_PLUS,
	ATM_QoS_ABR
};

/** struct atm_link_qos - ATM link QoS configuration */
struct atm_link_qos {
	/** QoS class */
	enum atm_qos_class qos_class;
	/** Specifies the upstream peak cell rate in cells per second */
	unsigned int peak_cell_rate;
	/** Specifies the upstream maximum burst size in cells */
	unsigned int max_burst_size;
	/** Specifies the upstream sustainable cell rate in cells per second */
	unsigned int sustainable_cell_rate;
};

/** struct atm_diag_loopback - ATM layer F5 OAM loopback test */
struct atm_diag_loopback {
	/** RW Number of repetitions of the ping test to perform before reporting the results */
	unsigned int num_repetition;
	/** RW Timeout in milliseconds for the ping test */
	unsigned int timeout;
	/** RO Number of successful ping tests */
	unsigned int success_count;
	/** RO Number of failed ping tests */
	unsigned int failure_count;
	/** RO Average response time in milliseconds of those successful ping tests */
	unsigned int avg_resp_time;
	/** RO Minimum response time in milliseconds of those successful ping tests */
	unsigned int min_resp_time;
	/** RO Maximum response time in milliseconds of those successful ping tests */
	unsigned int max_resp_time;
};

/** struct ptm_link - PTM link status */
struct ptm_link {
	/** The current operational status of the link */
	enum itf_status status;
	/** MAC address which is not necessarily the same as the Ethernet source of destination address */
	unsigned char mac_addr[6];
};

/**
 *  NOTE
 *
 *  There are NO PTM specific statistics since they are the same as a common IP interface's.
 */

/**
 * This function configures an ATM link
 *
 * @param[in] link_num - The ATM link number which starts with 0
 * @param[in] qos  - QoS settings
 * @param[in] link - The configuration parameters of the ATM link. Note that those RO (Read Only) members in
 *                  struct atm_link will be ignored
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int atm_configure(int link_num, const struct atm_link *link, const struct atm_link_qos *qos);

/**
 * This function gets the ATM link's information
 *
 * @param[in] link_num - The ATM link number which starts with 0
 * @param[out] link - The output parameter to receive the ATM link's configuration and status
 * @param[out] qos - The output parameter to receive the ATM QoS settings
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int atm_get_link_info(int link_num, struct atm_link *link, struct atm_link_qos *qos);

/**
 * This function gets the statistics of an ATM link
 *
 * @param[in] link_num - The ATM link number which starts with 0
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int atm_get_link_stats(int link_num, struct atm_link_stats *stats);

/**
 * This function starts or cancels a loopback test on an ATM link
 *
 * @param[in] link_num - The ATM link number which starts with 0
 * @param[in] loopback - Loopback test settings. Note that those RO (Read Only) members in
 *                       struct atm_diag_loopback will be ignored
 * @param[in] start - True to start a test. False to cancel an ongoing a test if any.
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int atm_loopback_test(int link_num, const struct atm_diag_loopback *loopback, bool start);

/**
 * This function gets the results of a loopback test on an ATM link
 *
 * @param[in] link_num - The ATM link number which starts with 0
 * @param[out] loopback - Output parameter to receive loopback test results
 *
 * @return The diagnostic state of a loopback test
 */
enum diag_state atm_get_loopback_results(int link_num, struct atm_diag_loopback *loopback);

/**
 * This function gets the PTM link's information
 *
 * @param[in] link_num - The PTM link number which starts with 0
 * @param[out] link - The output parameter to receive the PTM link's info
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int ptm_get_link_info(int link_num, struct ptm_link *link);

/**
 *  struct xtm_ops - This structure defines the ATM operations.
 *  A function pointer shall be NULL if the corresponding operation is not supported on a specific platform
 */
struct atm_ops {
	int (*configure)(int link_num, const struct atm_link *cfg, const struct atm_link_qos *qos);
	int (*get_link_info)(int link_num, struct atm_link *link, struct atm_link_qos *qos);
	int (*get_link_stats)(int link_num, struct atm_link_stats *stats);
	int (*loopback_test)(int link_num, const struct atm_diag_loopback *loopback, bool start);
	enum diag_state (*get_loopback_results)(int link_num, struct atm_diag_loopback *loopback);
};

/**
 *  struct xtm_ops - This structure defines the PTM operations.
 *  A function pointer shall be NULL if the corresponding operation is not supported on a specific platform
 */
struct ptm_ops {
	int (*get_link_info)(int link_num, struct ptm_link *link);
};

/** These global variables must be defined for each platform specific implementation */
extern const struct atm_ops atm_funcs;
extern const struct ptm_ops ptm_funcs;

#ifdef __cplusplus
}
#endif
#endif /* _XTM_H_ */
