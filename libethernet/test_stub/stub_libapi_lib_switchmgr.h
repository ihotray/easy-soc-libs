/*
 * stub_libapi_lib_switchmgr.h - dummy header with clean room Econet
 *                               definitions
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
#ifndef _STUB_LIBAPI_LIB_SWITCHMGR_H
#define _STUB_LIBAPI_LIB_SWITCHMGR_H

#include <inttypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ecnt_switchmgr_link_speed {
#define MAP_ECNT_LINK_SPEED(__name, __speed, __duplex) \
	__name,
#include "map_link_speed.def"
#undef MAP_ECNT_LINK_SPEED
} ecnt_switchmgr_link_speed;

/* Success state */
#define ECNT_SWITCHMGR_SUCCESS (0)

/* Link is up */
#define ECNT_SWITCHMGR_LINK_UP (1)
/* Link is down */
#define ECNT_SWITCHMGR_LINK_DOWN (0)
/* Return type */
#define ECNT_SWITCHMGR_RET int

typedef int ECNT_SWITCHMGR_LINK_STATE;
typedef int ECNT_SWITCHMGR_LINK_SPEED;

typedef struct ECNT_SWITCHMGR_PORT_STATISTICS {
	uint32_t TxDropFramesCnt;
	uint32_t TxBytesCnt_Lo;
	uint32_t TxPktsCnt;
	uint32_t TxBroadPktsCnt;
	uint32_t TxMultiPktsCnt;
	uint32_t TxUniPktsCnt;
	uint32_t TxCRCFramesCnt;
	uint32_t TxUnderSizePktsCnt;
	uint32_t TxOverSizePktsCnt;
	uint32_t Tx64BytePktsCnt;
	uint32_t Tx65_127BytePktsCnt;
	uint32_t Tx256_511BytePktsCnt;
	uint32_t Tx512_1023BytePktsCnt;
	uint32_t Tx1024_1518BytePktsCnt;
	uint32_t RxDropFramesCnt;
	uint32_t RxBytesCnt_Lo;
	uint32_t RxPktsCnt;
	uint32_t RxBroadPktsCnt;
	uint32_t RxMultiPktsCnt;
	uint32_t RxUniPktsCnt;
	uint32_t RxCRCFramesCnt;
	uint32_t RxUnderSizePktsCnt;
	uint32_t RxOverSizePktsCnt;
	uint32_t Rx64BytePktsCnt;
	uint32_t Rx65_127BytePktsCnt;
	uint32_t Rx256_511BytePktsCnt;
	uint32_t Rx512_1023BytePktsCnt;
	uint32_t Rx1024_1518BytePktsCnt;
} ECNT_SWITCHMGR_PORT_STATISTICS;

/* Function stubs */
int switchmgr_lib_get_port_statistics(uint8_t port,
									  ECNT_SWITCHMGR_PORT_STATISTICS *stats);
int switchmgr_lib_get_port_admin(uint8_t port, uint8_t *up);
int switchmgr_lib_get_port_autoneg_enable(uint8_t port, uint8_t *autoneg);
int switchmgr_lib_get_port_duplex(int port, char *duplex_mode);
int switchmgr_lib_get_port_max_bitrate(int port, char *speed_mode);
int switchmgr_lib_set_port_admin(uint8_t port, uint8_t up);

#ifdef __cplusplus
}
#endif

#endif /* _STUB_LIBAPI_LIB_SWITCHMGR_H */
