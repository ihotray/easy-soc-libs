/*
 * stub.h - 'test' ethernet module header
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

#ifndef TEST_ETHERNET_H
#define TEST_ETHERNET_H


#define TESTSTATS(attr)    test ## _ ## attr

#define GET_TEST_STATS(o, iface, attr, type)                         \
({                                                                      \
        memcpy(o, TESTSTATS(attr), sizeof(type));             \
})

const struct eth_stats test_eth_stats_data = {
        .tx_bytes = 100,
        .rx_bytes = 1200,
        .tx_packets = 900,
        .rx_packets = 100,
        .tx_errors = 10,
        .rx_errors = 20,
        .tx_ucast_packets = 1,
        .rx_ucast_packets = 2,
        .tx_mcast_packets = 3,
        .rx_mcast_packets = 4,
        .tx_bcast_packets = 5,
        .rx_bcast_packets = 6,
        .tx_discard_packets = 7,
        .rx_discard_packets = 8,
        .rx_unknown_packets = 9,
};

#define test_eth_stats       &test_eth_stats_data

const struct eth_rmon_stats test_eth_rmon_stats_data = {
        .tx.bytes = 230000,
        .tx.packets = 355000,
	.tx.bcast_packets = 2300,
        .tx.mcast_packets = 3,
	.tx.crc_err_packets = 43,
	.tx.under_sz_packets = 2,
	.tx.over_sz_packets = 300,
	.tx.packets_64bytes = 900000,
	.tx.packets_65to127bytes = 8200,
	.tx.packets_128to255bytes = 120000,
	.tx.packets_256to511bytes = 2400,
	.tx.packets_512to1023bytes = 100000,
	.tx.packets_1024to1518bytes = 27000,
        .rx.bytes = 12000,
        .rx.packets = 800000,
	.rx.bcast_packets = 3,
        .rx.mcast_packets = 4,
	.rx.crc_err_packets = 6000,
	.rx.under_sz_packets = 24,
	.rx.over_sz_packets = 4500,
	.rx.packets_64bytes = 6000,
	.rx.packets_65to127bytes = 41000,
	.rx.packets_128to255bytes = 3000000,
	.rx.packets_256to511bytes = 4500,
	.rx.packets_512to1023bytes = 560000,
	.rx.packets_1024to1518bytes = 34000,

};
#define test_eth_rmon_stats       &test_eth_rmon_stats_data
#endif /* TEST_ETHERNET_H */
