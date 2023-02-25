/*
 * bcm.c - implements APIs for Broadcom (X)DSL
 *
 * Copyright (C) 2019 iopsys Software Solutions AB. All rights reserved.
 *
 * Author: anjan.chanda@iopsys.eu
 *         yalu.zhang@iopsys.eu
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <stdbool.h>

typedef unsigned char   UINT8;
typedef unsigned short  UINT16, ushort;
typedef unsigned int    UINT32;
#include "adsldrv.h"
#include "AdslMibDef.h"

#include "common.h"
#include "xdsl.h"
#include "xtm.h"

#define BCM_DSL_DEV "/dev/bcmadsl0"

const struct dsl_ops xdsl_ops = {
	.get_line_info = dsl_get_line_info,
	.get_line_stats = dsl_get_line_stats,
	.get_line_stats_interval = dsl_get_line_stats_interval,
	.get_channel_info = dsl_get_channel_info,
	.get_channel_stats = dsl_get_channel_stats,
	.get_channel_stats_interval = dsl_get_channel_stats_interval,
	.get_fast_line_info = fast_get_line_info,
	.get_fast_line_stats = fast_get_line_stats,
	.get_fast_line_stats_interval = fast_get_line_stats_interval,
	.set_oem_parameter = dsl_set_oem_parameter,
};

static int max_line_num = 1;
static int max_chan_num = 1;

int dsl_get_line_number(void)
{
	return max_line_num;
}

int dsl_get_channel_number(void)
{
	return max_chan_num;
}

static inline int bcm_xdsl_dev_open(void)
{
	int retval = open(BCM_DSL_DEV, O_RDWR);

	if (retval < 0) {
		LIBDSL_LOG(LOG_ERR, "Failed to open DSL file %s, %s\n", BCM_DSL_DEV, strerror(errno));
	}

	return retval;
}

static inline int bcm_xdsl_dev_close(int fd)
{
	return close(fd);
}

static int bcm_xdsl_get_object(char *obj, int objlen, char *out, unsigned long *outlen)
{
	int fd;
	int retval;
	ADSLDRV_GET_OBJ arg;

	fd = bcm_xdsl_dev_open();
	if (fd < 0)
		return -1;

	arg.bvStatus = BCMADSL_STATUS_ERROR;
	arg.objId = obj;
	arg.objIdLen = objlen;
	arg.dataBuf = out;
	arg.dataBufLen = *outlen;

	retval = ioctl(fd, ADSLIOCTL_GET_OBJ_VALUE, &arg);
	*outlen = arg.dataBufLen;
	if (retval != 0 || arg.bvStatus == BCMADSL_STATUS_ERROR) {
		LIBDSL_LOG(LOG_ERR, "ioctl for ADSLIOCTL_GET_OBJ_VALUE failed, %s\n", strerror(errno));
		retval = -1;
	}

	bcm_xdsl_dev_close(fd);
	return retval;
}

static int bcm_xdsl_get_mib_info(adslMibInfo *adsl)
{
	unsigned long size = sizeof(adslMibInfo);
	int retval;

	retval = bcm_xdsl_get_object(NULL, 0, (char *)adsl, &size);
	if (retval < 0)
		return -1;

	return 0;
}

static int bcm_xdsl_get_cfg_info(adslCfgProfile *adsl)
{
	unsigned long size = sizeof(adslCfgProfile);
	int retval;
	char oidStr[] = { 95 };              /* kOidAdslPhyCfg */

	retval = bcm_xdsl_get_object(oidStr, sizeof(oidStr), (char *)adsl, &size);
	if (retval < 0)
		return -1;

	return 0;
}

static int bcm_xdsl_get_version(adslVersionInfo *dsl_ver)
{
	int fd, ret = 0;
	ADSLDRV_GET_VERSION arg;

	arg.pAdslVer = dsl_ver;
	arg.bvStatus = BCMADSL_STATUS_ERROR;

	fd = bcm_xdsl_dev_open();
	if (fd >= 0)
	{
		ioctl(fd, ADSLIOCTL_GET_VERSION, &arg);
		bcm_xdsl_dev_close(fd);

		if (arg.bvStatus == BCMADSL_STATUS_ERROR)
			ret = -1;
	} else
		ret = -1;

	return ret;
}

static int bcm_xdsl_set_oem_parameter(int paramId, void *buf, int bufLen)
{
	int fd, ret = 0;
	ADSLDRV_SET_OEM_PARAM arg;

	arg.paramId = paramId;
	arg.buf = buf;
	arg.len = bufLen;
	arg.bvStatus = BCMADSL_STATUS_ERROR;

	fd = bcm_xdsl_dev_open();
	if (fd >= 0)
	{
		ioctl(fd, ADSLIOCTL_SET_OEM_PARAM, &arg);
		bcm_xdsl_dev_close(fd);

		if (arg.bvStatus == BCMADSL_STATUS_ERROR)
			ret = -1;
	} else
		ret = -1;

	return ret;
}

int dsl_set_oem_parameter(int param_id, void *buf, int len) {
	return bcm_xdsl_set_oem_parameter(param_id, buf, len);
}

static int bcm_xdsl_get_annex(int mod_type, int xdsl_type)
{
	if (mod_type == kVdslModVdsl2) {
		return xdsl_type >> kXdslModeAnnexShift;
	} else if (xdsl_type & kAdsl2ModeAnnexMask) {
		int annex = kAdslTypeAnnexM;
		adslVersionInfo adsl_ver;

		if (bcm_xdsl_get_version(&adsl_ver) == 0 && kAdslTypeAnnexB == adsl_ver.phyType)
			annex = kAdslTypeAnnexJ;

		return annex;
	} else {
		return xdsl_type >> kXdslModeAnnexShift;
	}
}

int dsl_get_line_info(int line_num, struct dsl_line *line)
{
	adslMibInfo dsl_mib;
	adslCfgProfile adslCfg;
	UINT32 dslCfgParam;
	int i, number;
	bool is_vdsl;
	adslVersionInfo adsl_ver;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	memset(&adslCfg, 0, sizeof(adslCfg));
	if (bcm_xdsl_get_cfg_info(&adslCfg) != 0)
		return -1;

	// Initialize the output buffer
	memset(line, 0, sizeof(*line));

	/**
	 *  Convert the values
	 */
	is_vdsl = dsl_mib.adslConnection.modType == kVdslModVdsl2 ? true : false;
	// status according to TR-181_Issue-2_Amendment-14 Appendix XVI
	line->status = dsl_mib.adslPhys.adslCurrStatus == kAdslPhysStatusNoDefect &&
		dsl_mib.adslConnection.modType != kXdslModGfast ? IF_UP : IF_DOWN;
	line->upstream = true; /* This information can't be retrieved from anywhere */

	// firmware_version
	if (bcm_xdsl_get_version(&adsl_ver) == 0)
		strncpy(line->firmware_version, adsl_ver.phyVerStr, sizeof(line->firmware_version));

	if (adsl_ver.phyType == kAdslTypeAnnexC)
		dslCfgParam = adslCfg.adslAnnexCParam;
	else
		dslCfgParam = adslCfg.adslAnnexAParam;

	// Link status
	switch (dsl_mib.adslTrainingState) {
	case kAdslTrainingIdle:
		line->link_status = LINK_NOSIGNAL;
		break;
	case kAdslTrainingG994:
	case kAdslTrainingG992Started:
	case kAdslTrainingG993Started:
		line->link_status = LINK_INITIALIZING;
		break;
	case kAdslTrainingG992ChanAnalysis:
	case kAdslTrainingG992Exchange:
	case kAdslTrainingG993ChanAnalysis:
	case kAdslTrainingG993Exchange:
		line->link_status = LINK_ESTABLISHING;
		break;
	case kAdslTrainingConnected:
		line->link_status = LINK_UP;
		break;
	default:
		line->link_status = LINK_ERROR;
		break;
	}

	/* Note: Modify DSL link status as no signal while current mode is G.fast   */
	if ( dsl_mib.adslConnection.modType == kXdslModGfast )
		line->link_status = LINK_NOSIGNAL;

	/*
	 * XTSE
	 * Note that ASDL MIB supported by Broadcom is defined in RFC2662 and structure dsl_line is
	 * based on TR-181. These two standard have different levels of granularity so there is no
	 * one-to-one mapping between them.
	 *
	 * Assuming both overlapped and non overlapped mode is always enabled
	 * since kAdslCfgNoSpectrumOverlap is not set in driver
	 */
	line->standard_supported.use_xtse = true;
	if (dslCfgParam & kAdslCfgModT1413Only)
		XTSE_BIT_SET(line->standard_supported.xtse, T1_413);
	/* Note: bit 1 - no mention of ETSI_101_388 in driver */
	if (dslCfgParam & kAdslCfgModGdmtOnly) {
		switch (adsl_ver.phyType) {
		case kAdslTypeAnnexA:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_POTS_OVERLAPPED);
			break;
		case kAdslTypeAnnexB:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_ISDN_OVERLAPPED);
			break;
		case kAdslTypeAnnexC:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_TCM_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_1_TCM_ISDN_OVERLAPPED);
			break;
		}
	}
	if (dslCfgParam & kAdslCfgModGliteOnly) {
		switch (adsl_ver.phyType) {
		case kAdslTypeAnnexA:
			/* Note: the driver uses define G992P1_ANNEX_A_USED_FOR_G992P2
			 * for this which is currently only set for Annex A profiles
			 */
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_2_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_2_POTS_OVERLAPPED);
			break;
		case kAdslTypeAnnexC:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_2_TCM_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_2_TCM_ISDN_OVERLAPPED);
			break;
		}
	}
	if (dslCfgParam & kAdslCfgModAdsl2Only) {
		switch (adsl_ver.phyType) {
		case kAdslTypeAnnexA:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_OVERLAPPED);
			break;
		case kAdslTypeAnnexB:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_ISDN_OVERLAPPED);
			break;
		case kAdslTypeAnnexC:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_TCM_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_TCM_ISDN_OVERLAPPED);
			break;
		}
		/* Note: bit 25-26 - no mention of g.992.4 in driver */
		/* Note: bit 29-32 - no mention of g.992.3 Annex I/J in driver */
		/* Note: bit 33-34 - no mention of g.992.4 Annex I in driver */
		if (adslCfg.adsl2Param & kAdsl2CfgReachExOn) {
			if (adslCfg.adsl2Param & kAdsl2CfgAnnexLUpWide) {
				XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_MODE_1);
				XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_MODE_3);
			}
			if (adslCfg.adsl2Param & kAdsl2CfgAnnexLUpNarrow) {
				XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_MODE_2);
				XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_POTS_MODE_4);
			}
		}
		if (adslCfg.adsl2Param & kAdsl2CfgAnnexMp3) {
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_EXT_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_3_EXT_POTS_OVERLAPPED);
		}
	}
	if (dslCfgParam & kAdslCfgModAdsl2pOnly) {
		switch (adsl_ver.phyType) {
		case kAdslTypeAnnexA:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_POTS_OVERLAPPED);
			break;
		case kAdslTypeAnnexB:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_ISDN_OVERLAPPED);
			break;
		case kAdslTypeAnnexC:
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_TCM_ISDN_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_TCM_ISDN_OVERLAPPED);
			break;
		}
		/* Note: bit 47-50 - no mention of g.992.5 Annex I/J in driver */
		if (adslCfg.adsl2Param & kAdsl2CfgAnnexMp5) {
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_EXT_POTS_NON_OVERLAPPED);
			XTSE_BIT_SET(line->standard_supported.xtse, G_992_5_EXT_POTS_OVERLAPPED);
		}
	}
	if (dslCfgParam & (kDslCfgModVdsl2Only|kDslCfgModVdsl2LROnly)) {
		/* Note: only Annex A is defined for g.993.2 in driver */
		XTSE_BIT_SET(line->standard_supported.xtse, G_993_2_NORTH_AMERICA);
	}

	/*
	 * XTSUsed
	 *
	 * Only 1 bit allowed to be set. Assume overlapped until further notice.
	 */
	line->standard_used.use_xtse = true;
	if (line->status == IF_UP) {
		int annex;

		annex = bcm_xdsl_get_annex(dsl_mib.adslConnection.modType, dsl_mib.vdslInfo[0].vdsl2Mode);
		switch (dsl_mib.adslConnection.modType) {
		case kAdslModGdmt:
			switch (annex) {
			case kAdslTypeAnnexB:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_1_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexC:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_1_TCM_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexA:
			default:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_1_POTS_OVERLAPPED);
				break;
			}
			break;

		case kAdslModT1413:
			XTSE_BIT_SET(line->standard_used.xtse, T1_413);
			break;

		case kAdslModGlite:
			switch (annex) {
			case kAdslTypeAnnexC:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_2_TCM_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexA:
			default:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_2_POTS_OVERLAPPED);
				break;
			}
			break;

		case kAdslModAnnexI:
			/* Note: This should never appear since driver doesn't seem to support it */
			XTSE_BIT_SET(line->standard_used.xtse, G_992_3_ANNEX_I_OVERLAPPED);
			break;

		case kAdslModAdsl2:
		case kAdslModReAdsl2:
			switch (annex) {
			case kAdslTypeAnnexB:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexC:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_TCM_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexI:
				/* Note: This should never appear since driver doesn't seem to support it */
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_ANNEX_I_OVERLAPPED);
				break;
			case kAdslTypeAnnexJ:
				/* Note: This should never appear since driver doesn't seem to support it */
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_ANNEX_J_OVERLAPPED);
				break;
			case kAdslTypeAnnexL:
				if (dsl_mib.xdslInfo.xdslMode & kAdsl2ModeAnnexLUpWide)
					XTSE_BIT_SET(line->standard_used.xtse, G_992_3_POTS_MODE_3);
				else
					XTSE_BIT_SET(line->standard_used.xtse, G_992_3_POTS_MODE_4);
				break;
			case kAdslTypeAnnexM:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_EXT_POTS_OVERLAPPED);
				break;
			case kAdslTypeAnnexA:
			default:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_3_POTS_OVERLAPPED);
				break;
			}
			break;

		case kAdslModAdsl2p:
			switch (annex) {
			case kAdslTypeAnnexB:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexC:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_TCM_ISDN_OVERLAPPED);
				break;
			case kAdslTypeAnnexI:
				/* Note: This should never appear since driver doesn't seem to support it */
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_ANNEX_I_OVERLAPPED);
				break;
			case kAdslTypeAnnexJ:
				/* Note: This should never appear since driver doesn't seem to support it */
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_ANNEX_J_OVERLAPPED);
				break;
			case kAdslTypeAnnexM:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_EXT_POTS_OVERLAPPED);
				break;
			case kAdslTypeAnnexA:
			default:
				XTSE_BIT_SET(line->standard_used.xtse, G_992_5_POTS_OVERLAPPED);
				break;
			}
			break;

		case kVdslModVdsl2: // VDSL2
			switch (annex) {
			case kAdslTypeAnnexB:
				XTSE_BIT_SET(line->standard_used.xtse, G_993_2_EUROPE);
				break;
			case kAdslTypeAnnexC:
				XTSE_BIT_SET(line->standard_used.xtse, G_993_2_JAPAN);
				break;
			case kAdslTypeAnnexA:
			default:
				XTSE_BIT_SET(line->standard_used.xtse, G_993_2_NORTH_AMERICA);
				break;
			}
			break;

		default:
			break; // Invalid value
		}
	}

	// Line encoding
	switch (dsl_mib.adslLine.adslLineCoding) {
	case kAdslLineCodingDMT:
		line->line_encoding = LE_DMT;
		break;
	case kAdslLineCodingCAP:
		line->line_encoding = LE_CAP;
		break;
	case kAdslLineCodingQAM:
		line->line_encoding = LE_QAM;
		break;
	default:
		break; // Invalid value
	}

	// Allowed profiles
	if (dslCfgParam & kDslCfgModVdsl2Only) {
		if (adslCfg.vdslParam & kVdslProfile8a)
			line->allowed_profiles |= VDSL2_8a;
		if (adslCfg.vdslParam & kVdslProfile8b)
			line->allowed_profiles |= VDSL2_8b;
		if (adslCfg.vdslParam & kVdslProfile8c)
			line->allowed_profiles |= VDSL2_8c;
		if (adslCfg.vdslParam & kVdslProfile8d)
			line->allowed_profiles |= VDSL2_8d;
		if (adslCfg.vdslParam & kVdslProfile12a)
			line->allowed_profiles |= VDSL2_12a;
		if (adslCfg.vdslParam & kVdslProfile12b)
			line->allowed_profiles |= VDSL2_12b;
		if (adslCfg.vdslParam & kVdslProfile17a)
			line->allowed_profiles |= VDSL2_17a;
		if (adslCfg.vdslParam & kVdslProfile30a)
			line->allowed_profiles |= VDSL2_30a;
		if (adslCfg.vdslParam & kVdslProfile35b)
			line->allowed_profiles |= VDSL2_35b;
	}

	// Currently used profile, only applied for VDSL2
	if (is_vdsl) {
		switch (dsl_mib.vdslInfo[0].vdsl2Profile) {
		case kVdslProfile8a:
			line->current_profile = VDSL2_8a;
			break;
		case kVdslProfile8b:
			line->current_profile = VDSL2_8b;
			break;
		case kVdslProfile8c:
			line->current_profile = VDSL2_8c;
			break;
		case kVdslProfile8d:
			line->current_profile = VDSL2_8d;
			break;
		case kVdslProfile12a:
			line->current_profile = VDSL2_12a;
			break;
		case kVdslProfile12b:
			line->current_profile = VDSL2_12b;
			break;
		case kVdslProfile17a:
			line->current_profile = VDSL2_17a;
			break;
		case kVdslProfile30a:
			line->current_profile = VDSL2_30a;
			break;
		case kVdslProfile35b:
			line->current_profile = VDSL2_35b;
			break;
		default:
			break; // Invalid value
		}
	}

	// Power management state
	if (line->status == IF_UP && dsl_mib.vdslInfo[0].pwrState > 0 && dsl_mib.vdslInfo[0].pwrState <= 4) {
		line->power_management_state = DSL_L0 + dsl_mib.vdslInfo[0].pwrState;
	} else {
		line->power_management_state = DSL_L3; // No power
	}

	// Succes or failure cause of last initialization
	line->success_failure_cause = dsl_mib.xdslInitializationCause;

	// VTU-R estimated upstream power back-off electrical length
	line->upbokler = dsl_mib.xdslPhys.UPBOkle;   // which is which?

	// VTU-O estimated upstream power back-off electrical length per band
	line->upbokle_pb.count = dsl_mib.xdslAtucPhys.numKl0BandReported;
	for (i = 0; i < line->upbokle_pb.count; i++) {
		line->upbokle_pb.array[i] = dsl_mib.xdslAtucPhys.kl0PerBand[i];
	}

	// VTU-R estimated upstream power back-off electrical length per band
	line->upbokler_pb.count = dsl_mib.xdslPhys.numKl0BandReported;
	for (i = 0; i < line->upbokler_pb.count; i++) {
		line->upbokler_pb.array[i] = dsl_mib.xdslPhys.kl0PerBand[i];
	}

	// Downstream receiver signal level threshold
	line->rxthrsh_ds.count = 1;
	line->rxthrsh_ds.array[0] = dsl_mib.adslAlarm.adslThreshFastRateDown;

	// Actual signal-to-noise margin of the robust overhead channel (ROC)
	line->snr_mroc.us = dsl_mib.xdslAtucPhys.snrmRoc;
	line->snr_mroc.ds = dsl_mib.xdslPhys.snrmRoc;

	// The last successful transmitted initialization state in both directions
	line->last_state_transmitted.us = dsl_mib.xdslInitializationCause;
	line->last_state_transmitted.ds = dsl_mib.xdslInitializationCause;

	// VTU-O estimated upstream power back-off electrical length
	line->upbokle = dsl_mib.xdslPhys.UPBOkleCpe;   // which is which?

	// The allowed VDSL2 US0 PSD masks for Annex A operation
	line->us0_mask = kVdslUS0Mask;

	// Whether trellis coding is enabled in the downstream and upstream directions
	if(dsl_mib.adslConnection.modType < kAdslModAdsl2 ) {
		if(dsl_mib.adslConnection.trellisCoding ==kAdslTrellisOn) {
			line->trellis.us = 1;
			line->trellis.ds = 1;
		} else {
			line->trellis.us = 0;
			line->trellis.ds = 0;
		}
	} else {
		if (0 == (dsl_mib.adslConnection.trellisCoding2 & kAdsl2TrellisTxEnabled))
			line->trellis.us = 0;
		else
			line->trellis.us = 1;

		if(0 == (dsl_mib.adslConnection.trellisCoding2 & kAdsl2TrellisRxEnabled))
			line->trellis.ds = 0;
		else
			line->trellis.ds = 1;
	}

	// Virtual noise mode
	if (is_vdsl)
		line->act_snr_mode.us = line->act_snr_mode.ds = dsl_mib.xdslPhys.SNRmode;
	else
		line->act_snr_mode.us = line->act_snr_mode.ds = dsl_mib.adslPhys.SNRmode;

	if (line->status == IF_UP) {
		line->max_bit_rate.us = dsl_mib.adslAtucPhys.adslCurrAttainableRate / 1000;
		line->max_bit_rate.ds = dsl_mib.adslPhys.adslCurrAttainableRate/ 1000;

		line->noise_margin.us = dsl_mib.adslAtucPhys.adslCurrSnrMgn;
		line->noise_margin.ds = dsl_mib.adslPhys.adslCurrSnrMgn;
	}

	// Current signal-to-noise ratio margin of each upstream band
	number = sizeof(line->snr_mpb_us.array) / sizeof(line->snr_mpb_us.array[0]);
	line->snr_mpb_us.count = MAX_NUM_BANDS <= number ? MAX_NUM_BANDS : number;
	for (i = 0; i < line->snr_mpb_us.count; i++)
		line->snr_mpb_us.array[i] = dsl_mib.perbandDataUs[i].adslCurrSnrMgn;

	// Current signal-to-noise ratio margin of each downstream band
	number = sizeof(line->snr_mpb_ds.array) / sizeof(line->snr_mpb_ds.array[0]);
	line->snr_mpb_ds.count = MAX_NUM_BANDS <= number ? MAX_NUM_BANDS : number;
	for (i = 0; i < line->snr_mpb_ds.count; i++)
		line->snr_mpb_ds.array[i] = dsl_mib.perbandDataDs[i].adslCurrSnrMgn;

	line->attenuation.us = dsl_mib.adslAtucPhys.adslCurrAtn;
	line->attenuation.ds = dsl_mib.adslPhys.adslCurrAtn;

	line->power.us = dsl_mib.adslPhys.adslCurrOutputPwr;
	line->power.ds = dsl_mib.adslAtucPhys.adslCurrOutputPwr;

	// XTU-R vendor ID (bytes 2-5 of adslVendorId)
	for (i = 0; i < 4; i++)
		sprintf(line->xtur_vendor + i * 2, "%02X", dsl_mib.adslPhys.adslVendorID[i + 2]);

	// XTU-C vendor ID (bytes 2-5 of adslVendorId)
	for (i = 0; i < 4; i++)
		sprintf(line->xtuc_vendor + i * 2, "%02X", dsl_mib.adslAtucPhys.adslVendorID[i + 2]);

	// XTC-C vendor revision number
	line->xtuc_ansi_rev = (unsigned int)strtoul(dsl_mib.adslAtucPhys.adslVersionNumber, NULL, 0);

	/*
	 * The following parameters are not supported on Broadcom platform. Dummy values are assigned
	 * as per the meaning of the parameters accordingly.
	 */
	/* TODO: this may be gotten from adslCfgProfile */
	line->act_ra_mode.us = line->act_ra_mode.ds = 1; // Actual active rate adaptation mode
	/* TODO: this can be gotten from adslCfgProfile */
	line->line_number = 1;  // 1 = innermost pair
	strncpy(line->xtur_country, "0000", sizeof(line->xtur_country)); // XTU-R country
	line->xtur_ansi_std = line->xtur_ansi_rev = 0; // XTU-R revision number and vendor revision number
	strncpy(line->xtuc_country, "0000", sizeof(line->xtuc_country)); // XTU-C country
	line->xtuc_ansi_std = 0; // XTU-C revision number

	return 0;
}

int dsl_get_line_stats(int line_num, struct dsl_line_channel_stats *stats)
{
	adslMibInfo dsl_mib;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Convert the values
	 */
	if (!( dsl_mib.adslConnection.modType == kXdslModGfast )) {
		stats->total_start = dsl_mib.adslPerfData.adslSinceDrvStartedTimeElapsed;
		stats->showtime_start = dsl_mib.adslPerfData.adslSinceLinkTimeElapsed;
		stats->last_showtime_start = dsl_mib.adslPerfData.adslSincePrevLinkTimeElapsed;
		stats->current_day_start = dsl_mib.adslPerfData.adslPerfCurr1DayTimeElapsed;
		stats->quarter_hour_start = dsl_mib.adslPerfData.adslPerfCurr15MinTimeElapsed;
	}

	return 0;
}

int dsl_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct dsl_line_stats_interval *stats)
{
	adslMibInfo dsl_mib;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Convert the values
	 */
	if (!( dsl_mib.adslConnection.modType == kXdslModGfast )) {
		switch (type) {
		case DSL_STATS_TOTAL:
			stats->errored_secs = dsl_mib.adslPerfData.perfTotal.adslESs;
			stats->severely_errored_secs = dsl_mib.adslPerfData.perfTotal.adslSES;
			break;
		case DSL_STATS_SHOWTIME:
			stats->errored_secs = dsl_mib.adslPerfData.perfSinceShowTime.adslESs;
			stats->severely_errored_secs = dsl_mib.adslPerfData.perfSinceShowTime.adslSES;
			break;
		case DSL_STATS_LASTSHOWTIME: // N/A
			stats->errored_secs = DSL_INVALID_STATS_COUNTER;
			stats->severely_errored_secs = DSL_INVALID_STATS_COUNTER;
			break;
		case DSL_STATS_CURRENTDAY:
			stats->errored_secs = dsl_mib.adslPerfData.perfCurr1Day.adslESs;
			stats->severely_errored_secs = dsl_mib.adslPerfData.perfCurr1Day.adslSES;
			break;
		case DSL_STATS_QUARTERHOUR:
			stats->errored_secs = dsl_mib.adslPerfData.perfCurr15Min.adslESs;
			stats->severely_errored_secs = dsl_mib.adslPerfData.perfCurr15Min.adslSES;
			break;
		default:
			LIBDSL_LOG(LOG_ERR, "Unknown interval type for DSL line statistics, %d\n", type);
			return -1;
		}
	}

	return 0;
}

static int bcm_get_vdsl2_aggr_rate(int profile, xdslFramingInfo *frame_info)
{
	int       phy_adjustment = (1 == frame_info->rtxMode) ? 1 : 0;
	long long num = (long long)1024 * frame_info->L * (frame_info->N - frame_info->R - phy_adjustment);

	if (kVdslProfile30a == profile)
		num <<= 1;

    return (0 == frame_info->N) ? -1 : num / (frame_info->N * 257);
}

static int bcm_get_adsl_aggr_rate(xdslFramingInfo *frame_info)
{
	int       phy_adjustment = (1 == frame_info->rtxMode) ? 1 : 0;
	long long num = (long long)1024 * frame_info->L * (frame_info->N - frame_info->R - phy_adjustment);

    return (0 == frame_info->N) ? -1 : num / (frame_info->N * 257);
}

static unsigned int bcm_xdsl_get_aggr_rate(bool is_vdsl2, int profile, xdslFramingInfo *frame_info)
{
	if (is_vdsl2)
		return bcm_get_vdsl2_aggr_rate(profile, frame_info);
	else
		return bcm_get_adsl_aggr_rate(frame_info);
}

int dsl_get_channel_info(int chan_num, struct dsl_channel *channel)
{
	adslMibInfo dsl_mib;
	bool is_vdsl;

	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(channel, 0, sizeof(*channel));

	/**
	 *  Convert the values
	 */
	is_vdsl = dsl_mib.adslConnection.modType == kVdslModVdsl2 ? true : false;
	// status according to TR-181_Issue-2_Amendment-14 Appendix XVI
	channel->status = dsl_mib.adslPhys.adslCurrStatus == kAdslPhysStatusNoDefect &&
		dsl_mib.adslConnection.modType != kXdslModGfast ? IF_UP : IF_LLDOWN;

	// Link encapsulation supported and currently used
	channel->link_encapsulation_supported = G_992_3_ANNEK_K_ATM | G_992_3_ANNEK_K_PTM |
			G_993_2_ANNEK_K_ATM | G_993_2_ANNEK_K_PTM | G_994_1_AUTO;

	switch (dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].tmType[0]) {
	case kXdslDataPtm:
		channel->link_encapsulation_used = is_vdsl ? G_993_2_ANNEK_K_PTM : G_992_3_ANNEK_K_PTM;
		break;
	case kXdslDataAtm:
	case kXdslDataNitro:
		channel->link_encapsulation_used = is_vdsl ? G_993_2_ANNEK_K_ATM : G_992_3_ANNEK_K_ATM;
		break;
	case kXdslDataRaw:
		channel->link_encapsulation_used = G_994_1_AUTO;
		break;
	default:
		// Invalid value
		break;
	}

	// Latency path information
	channel->lpath = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].pathId;
	channel->intlvdepth = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].D;
	channel->intlvblock = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].I;
	channel->actual_interleaving_delay = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].delay;
	channel->actinp = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].INP;

	// Reed-Solomon (RS) codeword size
	channel->nfec = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].N;
	channel->lsymb = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].L;

	// Physical layer aggregate data rate
	channel->curr_rate.us = bcm_xdsl_get_aggr_rate(is_vdsl, dsl_mib.vdslInfo[0].vdsl2Profile,
			&dsl_mib.xdslInfo.dirInfo[1].lpInfo[0]);
	channel->curr_rate.ds = bcm_xdsl_get_aggr_rate(is_vdsl, dsl_mib.vdslInfo[0].vdsl2Profile,
			&dsl_mib.xdslInfo.dirInfo[0].lpInfo[0]);

	// Actual net data rate
	channel->actndr.us = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].dataRate;
	channel->actndr.ds = dsl_mib.xdslInfo.dirInfo[0].lpInfo[0].dataRate;

	// Actual impulse noise protection again REIN
	channel->actinprein.us = dsl_mib.xdslInfo.dirInfo[1].lpInfo[0].INPrein;
	channel->actinprein.ds = dsl_mib.xdslInfo.dirInfo[0].lpInfo[0].INPrein;

	/*
	 * The following parameters are not supported on Broadcom platform. Dummy values are assigned
	 * as per the meaning of the parameters accordingly.
	 */
	channel->inpreport = false;
	channel->rfec = -1;

	return 0;
}

int dsl_get_channel_stats(int chan_num, struct dsl_line_channel_stats *stats)
{
	adslMibInfo dsl_mib;

	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	adslChanPerfDataEntry	*pChanPerfData;
	if(!dsl_mib.lp2Active && !dsl_mib.lp2TxActive) {
		if (kAdslIntlChannel == dsl_mib.adslConnection.chType) {
			pChanPerfData = &dsl_mib.adslChanIntlPerfData;
		} else {
			pChanPerfData = &dsl_mib.adslChanFastPerfData;
		}
	} else {
		/* path/bear 0 for G.inp case */
		pChanPerfData = &dsl_mib.xdslChanPerfData[0];
	}

	/**
	 *  Convert the values
	 */
	stats->current_day_start = pChanPerfData->adslPerfCurr1DayTimeElapsed;
	stats->quarter_hour_start = pChanPerfData->adslPerfCurr15MinTimeElapsed;

	/*
	 * The following parameters are not supported on Broadcom platform. Dummy values are assigned
	 * as per the meaning of the parameters accordingly.
	 */
	stats->total_start = 0;
	stats->showtime_start = 0;
	stats->last_showtime_start = 0;

	return 0;
}

int dsl_get_channel_stats_interval(int chan_num, enum dsl_stats_type type, struct dsl_channel_stats_interval *stats)
{
	adslMibInfo dsl_mib;

	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	stats->xtur_fec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_fec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtur_hec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_hec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtur_crc_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_crc_errors = DSL_INVALID_STATS_COUNTER;

	adslChanPerfDataEntry	*pChanPerfData;
	if(!dsl_mib.lp2Active && !dsl_mib.lp2TxActive) {
		if (kAdslIntlChannel == dsl_mib.adslConnection.chType) {
			pChanPerfData = &dsl_mib.adslChanIntlPerfData;
		} else {
			pChanPerfData = &dsl_mib.adslChanFastPerfData;
		}
	} else {
		/* path/bear 0 for G.inp case */
		pChanPerfData = &dsl_mib.xdslChanPerfData[0];
	}

	/**
	 *  Convert the values
	 */
	switch (type) {
	case DSL_STATS_TOTAL:
		stats->xtur_fec_errors = pChanPerfData->perfTotal.adslChanTxFEC;
		stats->xtuc_fec_errors = pChanPerfData->perfTotal.adslChanCorrectedBlks;
		stats->xtur_crc_errors = pChanPerfData->perfTotal.adslChanTxCRC;
		stats->xtuc_crc_errors = pChanPerfData->perfTotal.adslChanUncorrectBlks;
		break;
	case DSL_STATS_SHOWTIME:
		stats->xtur_fec_errors = dsl_mib.adslStat.xmtStat.cntRSCor;
		stats->xtuc_fec_errors = dsl_mib.adslStat.rcvStat.cntRSCor;
		/*
		 * HEC counters actually are not included in any periodical statistics. But driver source shows that
		 * it is in the category of showtime. And it also complies to the test results by Telia Eesti in
		 * ticket #1043.
		 */
		stats->xtur_hec_errors = dsl_mib.atmStat2lp[0].xmtStat.cntHEC;
		stats->xtuc_hec_errors = dsl_mib.atmStat2lp[0].rcvStat.cntHEC;
		stats->xtur_crc_errors = dsl_mib.adslStat.xmtStat.cntSFErr;
		stats->xtuc_crc_errors = dsl_mib.adslStat.rcvStat.cntSFErr;
		break;
	case DSL_STATS_LASTSHOWTIME: // N/A
		break;
	case DSL_STATS_CURRENTDAY:
		stats->xtur_fec_errors = pChanPerfData->perfCurr1Day.adslChanTxFEC;
		stats->xtuc_fec_errors = pChanPerfData->perfCurr1Day.adslChanCorrectedBlks;
		stats->xtur_crc_errors = pChanPerfData->perfCurr1Day.adslChanTxCRC;
		stats->xtuc_crc_errors = pChanPerfData->perfCurr1Day.adslChanUncorrectBlks;
		break;
	case DSL_STATS_QUARTERHOUR:
		stats->xtur_fec_errors = pChanPerfData->perfCurr15Min.adslChanTxFEC;
		stats->xtuc_fec_errors = pChanPerfData->perfCurr15Min.adslChanCorrectedBlks;
		stats->xtur_crc_errors = pChanPerfData->perfCurr15Min.adslChanTxCRC;
		stats->xtuc_crc_errors = pChanPerfData->perfCurr15Min.adslChanUncorrectBlks;
		break;
	default:
		LIBDSL_LOG(LOG_ERR, "Unknown interval type for DSL channel statistics, %d\n", type);
		return -1;
	}

	return 0;
}

int fast_get_line_info(int line_num, struct fast_line *line)
{
	adslMibInfo dsl_mib;
	adslCfgProfile adslCfg;
	UINT32 dslCfgParam;
	adslVersionInfo adsl_ver;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	memset(&adslCfg, 0, sizeof(adslCfg));
	if (bcm_xdsl_get_cfg_info(&adslCfg) != 0)
		return -1;

	// Initialize the output buffer
	memset(line, 0, sizeof(*line));

	/**
	 *  Convert the values
	 */
	// status according to TR-181_Issue-2_Amendment-14 Appendix XVI
	line->status = dsl_mib.adslPhys.adslCurrStatus	== kAdslPhysStatusNoDefect &&
		dsl_mib.adslConnection.modType == kXdslModGfast ? IF_UP : IF_DOWN;
	line->upstream = true; /* This information can't be retrieved from anywhere */

	// firmware_version
	if (bcm_xdsl_get_version(&adsl_ver) == 0)
		strncpy(line->firmware_version, adsl_ver.phyVerStr, sizeof(line->firmware_version));

	if (adsl_ver.phyType == kAdslTypeAnnexC)
		dslCfgParam = adslCfg.adslAnnexCParam;
	else
		dslCfgParam = adslCfg.adslAnnexAParam;

	// Link status
	switch (dsl_mib.adslTrainingState) {
	case kAdslTrainingIdle:
		line->link_status = LINK_NOSIGNAL;
		break;
	case kAdslTrainingG994:
	case kAdslTrainingG992Started:
	case kAdslTrainingG993Started:
		line->link_status = LINK_INITIALIZING;
		break;
	case kAdslTrainingG992ChanAnalysis:
	case kAdslTrainingG992Exchange:
	case kAdslTrainingG993ChanAnalysis:
	case kAdslTrainingG993Exchange:
		line->link_status = LINK_ESTABLISHING;
		break;
	case kAdslTrainingConnected:
		line->link_status = LINK_UP;
		break;
	default:
		line->link_status = LINK_ERROR;
		break;
	}

	// Allowed profiles
	if (dslCfgParam & kDslCfgModGfastOnly) {
		// Note: These are inverted
		if (!(adslCfg.vdslParam & kGfastProfile106aDisable))
			line->allowed_profiles |= FAST_106a;
		if (!(adslCfg.vdslParam & kGfastProfile106bDisable))
			line->allowed_profiles |= FAST_106b;
#ifndef CONFIG_BCM963138
		if (!(adslCfg.vdslParam & kGfastProfile212aDisable))
			line->allowed_profiles |= FAST_212a;
		if (!(adslCfg.vdslParam & kGfastProfile106cDisable))
			line->allowed_profiles |= FAST_106c;
		if (!(adslCfg.vdslParam & kGfastProfile212cDisable))
			line->allowed_profiles |= FAST_212c;
#endif
	}

	// Currently used profile
	switch (dsl_mib.xdslInfo.vdsl2Profile) {
	case kGfastProfile106a:
		line->current_profile = FAST_106a;
		break;
	case kGfastProfile212a:
		line->current_profile = FAST_212a;
		break;
	case kGfastProfile106b:
		line->current_profile = FAST_106b;
		break;
	case kGfastProfile106c:
		line->current_profile = FAST_106c;
		break;
	case kGfastProfile212c:
		line->current_profile = FAST_212c;
		break;
	default:
		break; // Invalid value
	}

	// Power management state
	if (line->status == IF_UP && dsl_mib.xdslInfo.pwrState > 0 && dsl_mib.xdslInfo.pwrState <= 4) {
		line->power_management_state = DSL_L0 + dsl_mib.xdslInfo.pwrState;
	} else {
		line->power_management_state = DSL_L3; // No power
	}

	// Succes or failure cause of last initialization
	line->success_failure_cause = dsl_mib.xdslInitializationCause;

	// FTU-R estimated upstream power back-off electrical length
	line->upbokler = dsl_mib.xdslPhys.UPBOkle;

	// FTU-O estimated upstream power back-off electrical length
	line->upbokle = dsl_mib.xdslPhys.UPBOkleCpe;

	line->max_bit_rate.us = dsl_mib.adslAtucPhys.adslCurrAttainableRate / 1000;
	line->max_bit_rate.ds = dsl_mib.adslPhys.adslCurrAttainableRate/ 1000;

	line->noise_margin.us = dsl_mib.adslAtucPhys.adslCurrSnrMgn;
	line->noise_margin.ds = dsl_mib.adslPhys.adslCurrSnrMgn;

	line->attenuation.us = dsl_mib.adslAtucPhys.adslCurrAtn;
	line->attenuation.ds = dsl_mib.adslPhys.adslCurrAtn;

	line->power.us = dsl_mib.adslPhys.adslCurrOutputPwr;
	line->power.ds = dsl_mib.adslAtucPhys.adslCurrOutputPwr;

	line->snrm_rmc.us = dsl_mib.xdslAtucPhys.snrmRoc;
	line->snrm_rmc.ds = dsl_mib.xdslPhys.snrmRoc;

	/*
	 * The following parameters are not supported on Broadcom platform. Dummy values are assigned
	 * as per the meaning of the parameters accordingly.
	 */
	line->last_transmitted_signal.us = line->last_transmitted_signal.ds = 0;
	/* TODO: this can be gotten from adslCfgProfile */
	line->line_number = 1;  // 1 = innermost pair
	/* TODO: perhaps dsl_mib.xdslInfo.dirInfo[0|1].lpInfo[0].Lrcm can be
	 * used for bits_rmc_ps_ds/bits_rmc_ps_us */
	line->fext_to_cancel_enable.us = line->fext_to_cancel_enable.ds = 0;

	return 0;
}

int fast_get_line_stats(int line_num, struct fast_line_stats *stats)
{
	adslMibInfo dsl_mib;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Convert the values
	 */
	stats->total_start = dsl_mib.adslPerfData.adslSinceDrvStartedTimeElapsed;
	stats->showtime_start = dsl_mib.adslPerfData.adslSinceLinkTimeElapsed;
	stats->last_showtime_start = dsl_mib.adslPerfData.adslSincePrevLinkTimeElapsed;
	stats->current_day_start = dsl_mib.adslPerfData.adslPerfCurr1DayTimeElapsed;
	stats->quarter_hour_start = dsl_mib.adslPerfData.adslPerfCurr15MinTimeElapsed;

	return 0;
}

int fast_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct fast_line_stats_interval *stats)
{
	adslMibInfo dsl_mib;
	adslPerfCounters *pPerfCnt = NULL;
	rtxCounters *pRtxCnt = NULL;
	gfastOlrCounters *pGfastOlrCnt = NULL;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Get the data
	memset(&dsl_mib, 0, sizeof(dsl_mib));
	if (bcm_xdsl_get_mib_info(&dsl_mib) != 0)
		return -1;

	// Initialize the output buffer
	memset(stats, -1, sizeof(*stats));  // -1 invalidates all counters

	/**
	 *  Convert the values
	 */
	switch (type) {
	case DSL_STATS_TOTAL:
		pPerfCnt = &dsl_mib.adslPerfData.perfTotal;
		pRtxCnt = &dsl_mib.rtxCounterData.cntDS.perfTotal;
		pGfastOlrCnt = &dsl_mib.gfastOlrCounterData.cntDS.perfTotal;
		break;
	case DSL_STATS_SHOWTIME:
		pPerfCnt = &dsl_mib.adslPerfData.perfSinceShowTime;
		pRtxCnt = &dsl_mib.rtxCounterData.cntDS.perfSinceShowTime;
		pGfastOlrCnt = &dsl_mib.gfastOlrCounterData.cntDS.perfSinceShowTime;
		break;
	case DSL_STATS_LASTSHOWTIME: // N/A
		// Counters already invalidated by memset
		return 0;
	case DSL_STATS_CURRENTDAY:
		pPerfCnt = &dsl_mib.adslPerfData.perfCurr1Day;
		pRtxCnt = &dsl_mib.rtxCounterData.cntDS.perfCurr1Day;
		pGfastOlrCnt = &dsl_mib.gfastOlrCounterData.cntDS.perfCurr1Day;
		break;
	case DSL_STATS_QUARTERHOUR:
		pPerfCnt = &dsl_mib.adslPerfData.perfCurr15Min;
		pRtxCnt = &dsl_mib.rtxCounterData.cntDS.perfCurr15Min;
		pGfastOlrCnt = &dsl_mib.gfastOlrCounterData.cntDS.perfCurr15Min;
		break;
	default:
		LIBDSL_LOG(LOG_ERR, "Unknown interval type for DSL line statistics, %d\n", type);
		return -1;
	}

	stats->errored_secs = pPerfCnt->adslESs;
	stats->severely_errored_secs = pPerfCnt->adslSES;
	stats->loss = pPerfCnt->adslLoss;
	stats->lors = pPerfCnt->xdslLors;
	stats->uas = pPerfCnt->adslUAS;

	stats->rtx_uc = pRtxCnt->rtx_uc;
	stats->rtx_tx = pRtxCnt->rtx_tx;

	stats->success_bsw = pGfastOlrCnt->bswCompleted;
	stats->success_sra = pGfastOlrCnt->sraCompleted;
	stats->success_fra = pGfastOlrCnt->fraCompleted;
	stats->success_rpa = pGfastOlrCnt->rpaCompleted;
	stats->success_tiga = pGfastOlrCnt->tigaCompleted;

	return 0;
}

const struct atm_ops atm_funcs = {};
const struct ptm_ops ptm_funcs = {};

