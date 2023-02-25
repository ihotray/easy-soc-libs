/*
 * ecnt_prvt.h - header for Econet switch private utilities
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
#ifndef _ECNT_PRVT_H
#define _ECNT_PRVT_H

#include <inttypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Invalid port number */
#define ECNT_PRVT_PORT_NUM_INVALID 	(UINT32_MAX)

/** Get switch port number from the network interface name
 *
 * Usage:
 *
 * ecnt_prvt_get_port_num("eth0.1");
 * ecnt_prvt_get_port_num("nas10");
 * ecnt_prvt_get_port_num("ae_wan");
 *
 * @param[in] ifname interface name.
 * @return Arbitrary number on success, @c ECNT_PRVT_PORT_NUM_INVALID on error.
 */
uint32_t ecnt_prvt_get_port_num(const char *ifname);

/** Convert link speed from Econet's enumeration to libethernet's internal
 *  representation.
 *
 * Usage:
 *
 * ecnt_prvt_link_speed2lib(ecnt_prvt_link_speed2lib);
 *
 * @param[in] link_speed Link speed with the value from Econet's enumeration.
 * @return Link speed in eth_speed enumeration form.
 *
 * @remarks link_speed is intentionally left with generic type to avoid
 *          introduction of vendor dependencies.
 */
enum eth_speed ecnt_prvt_link_speed2lib(int link_speed);

/** Get switch port's link settings.
 *
 * Usage:
 *
 * struct eth_link link_settings;
 * ecnt_prvt_get_link_settings(1, &link_settings);
 *
 * @param[in]     port_num Port number relative to switch's port map.
 * @param[in,out] link     Pointer to structure receiving link settings.
 * @return @c 0 on success, @c -1 on error.
 */
int ecnt_prvt_get_link_settings(uint32_t port_num, struct eth_link *link);

/** Get switch port's statistic data.
 *
 * Usage:
 *
 * struct eth_stats stats;
 * struct eth_rmon_stats rmon_stats;
 * ecnt_prvt_get_port_statistics(1, &stats, &rmon_stats);
 *
 * ecnt_prvt_get_port_statistics(2, &stats, NULL);
 *
 * ecnt_prvt_get_port_statistics(3, NULL, &rmon_stats);
 *
 * @param[in]     port_num   Port number relative to switch's port map.
 * @param[in,out] stats      Pointer to structure receiving ethernet interface
 *                           statistics (as defined by @p eth_stats structure).
 * @param[in,out] rmon_stats Pointer to structure receiving ethernet interface
 *                           rmon statistics (as defined by @p eth_rmon_stats
 *                           structure).
 * @return @c 0 on success, @c -1 on error.
 */
int ecnt_prvt_get_port_statistics(uint32_t port_num,
                                  struct eth_stats *stats,
                                  struct eth_rmon_stats *rmon_stats);

/** Set switch port's state
 *
 * Usage:
 *
 * ecnt_prvt_set_port_state(2, true);
 *
 * ecnt_prvt_set_port_state(1, false);
 *
 * @param[in] port_num   Port number relative to switch's port map.
 * @param[in] state      @c true to enable port, @c false to disable it.
 * @return @c 0 on success, @c -1 on error.
 */
int ecnt_prvt_set_port_state(uint32_t port_num, bool state);

#ifdef __cplusplus
}
#endif

#endif /* _ECNT_PRVT_H */
