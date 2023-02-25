/*
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
#include <stdio.h>
#include <stdlib.h>
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
#include <syslog.h>
#include <time.h>

#include "xdsl.h"
#include "xtm.h"

#define LIBDSL_LOG_INPUT_PARAMS "/tmp/libdsl-input-params.log"

const struct dsl_ops xdsl_ops = {
	.get_line_info = dsl_get_line_info,
	.get_line_stats = dsl_get_line_stats,
	.get_line_stats_interval = dsl_get_line_stats_interval,
	.get_channel_info = dsl_get_channel_info,
	.get_channel_stats = dsl_get_channel_stats,
	.get_channel_stats_interval = dsl_get_channel_stats_interval,

	.get_line_test_params = dsl_get_line_test_params,
	.get_line_data_gathering = dsl_get_line_data_gathering,

	.line_test = adsl_line_test,
	.get_line_test_results = adsl_get_line_test_results,

	.seltuer_test = dsl_seltuer_test,
	.get_seltuer_results = dsl_get_seltuer_results,

	.seltqln_test = dsl_seltqln_test,
	.get_seltqln_results = dsl_get_seltqln_results,

	.seltp_test = dsl_seltp_test,
	.get_seltp_results = dsl_get_seltp_results,

	.get_fast_line_info = fast_get_line_info,
	.get_fast_line_stats = fast_get_line_stats,
	.get_fast_line_stats_interval = fast_get_line_stats_interval,
	.get_fast_line_test_params = fast_get_line_test_params,

	.configure = dsl_configure
};

// TODO: this needs to be updated when supporting DSL bonding
static int max_line_num = 1;
static int max_chan_num = 1;

static void log_test(const char *fmt, ...)
{
	FILE *fp = NULL;
	va_list args;

	fp = fopen(LIBDSL_LOG_INPUT_PARAMS, "a");
	if (!fp)
		return;

	time_t now = time(NULL);
	struct tm *tm_now = localtime(&now);
	const char *tm_fmt = "[%4d-%02d-%02d %02d:%02d:%02d] ";

	va_start(args, fmt);
	fprintf(fp, tm_fmt,		/* Flawfinder: ignore */
			tm_now->tm_year + 1900,
			tm_now->tm_mon + 1,
			tm_now->tm_mday,
			tm_now->tm_hour,
			tm_now->tm_min,
			tm_now->tm_sec);
	vfprintf(fp, fmt, args);	/* Flawfinder: ignore */
	va_end(args);

	fflush(fp);
	fclose(fp);
}

int dsl_get_line_number(void)
{
	return max_line_num;
}

int dsl_get_channel_number(void)
{
	return max_chan_num;
}

int dsl_get_line_info(int line_num, struct dsl_line *line)
{
	int i;
	bool is_vdsl = true;

	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Initialize the output buffer
	memset(line, 0, sizeof(*line));

	/**
	 *  Fill in fake values by the output of libdsl_test utility
	 */
	line->status = IF_UP;
	line->upstream = true;
	strncpy(line->firmware_version, "A2pvfbH045k.d27f", sizeof(line->firmware_version) - 1);
	line->link_status = LINK_UP;

	line->standard_supported.use_xtse = false;
	line->standard_supported.mode = MOD_G_922_1_ANNEX_A | MOD_G_922_1_ANNEX_B | MOD_G_922_1_ANNEX_C | // Gdmt
			MOD_G_992_2 | // Glite
			MOD_T1_413 | MOD_T1_413i2 | // T1413
			MOD_G_992_3_Annex_A | MOD_G_992_3_Annex_B | MOD_G_992_3_Annex_C | MOD_G_992_3_Annex_I |
			MOD_G_992_3_Annex_J | MOD_G_992_3_Annex_L | MOD_G_992_3_Annex_M | // ADSL2
			MOD_G_992_5_Annex_A | MOD_G_992_5_Annex_B | MOD_G_992_5_Annex_C |
			MOD_G_992_5_Annex_I | MOD_G_992_5_Annex_J | MOD_G_992_5_Annex_M | // ADSL2+
			MOD_G_993_2_Annex_A | MOD_G_993_2_Annex_B | MOD_G_993_2_Annex_C;  // VDSL2

	/*
	 * standard_used
	 */
	line->standard_used.use_xtse = false;
	if (line->status == IF_UP)
		line->standard_used.mode = MOD_G_993_2_Annex_A;

	// Line encoding
	line->line_encoding = LE_DMT;

	line->allowed_profiles = VDSL2_8a | VDSL2_8b | VDSL2_8c | VDSL2_8d |
			VDSL2_12a | VDSL2_12b | VDSL2_17a | VDSL2_30a | VDSL2_35b;

	// Currently used profile, only applied for VDSL2
	if (is_vdsl) {
		line->current_profile = VDSL2_17a;
	}

	// Power management state
	line->power_management_state = DSL_L0;

	// VTU-R estimated upstream power back-off electrical length
	line->upbokler_pb.count = 1;
	line->upbokler_pb.array[0] = 180;

	// Downstream receiver signal level threshold
	line->rxthrsh_ds.count = 1;
	line->rxthrsh_ds.array[0] = 0;

	// The last successful transmitted initialization state in both directions
	line->last_state_transmitted.us = 0;
	line->last_state_transmitted.ds = 0;

	// The allowed VDSL2 US0 PSD masks for Annex A operation
	line->us0_mask = 0x7f0000;

	// Whether trellis coding is enabled in the downstream and upstream directions
	line->trellis.us = 1;
	line->trellis.ds = 1;

	// Virtual noise mode
	line->act_snr_mode.us = line->act_snr_mode.ds = 1;

	if (line->status == IF_UP) {
		line->max_bit_rate.us = 65329;
		line->max_bit_rate.ds = 148444;

		line->noise_margin.us = 146;
		line->noise_margin.ds = 177 ;
	}

	// Current signal-to-noise ratio margin of each upstream band
	line->snr_mpb_us.count = 4;
	{
		long tmp[4] = {127,183,131,0};
		for (i = 0; i < line->snr_mpb_us.count; i++)
			line->snr_mpb_us.array[i] = tmp[i];
	}

	// Current signal-to-noise ratio margin of each downstream band
	line->snr_mpb_ds.count = 4;
	{
		long tmp[4] = {176,177,177,0};
		for (i = 0; i < line->snr_mpb_ds.count; i++)
			line->snr_mpb_ds.array[i] = tmp[i];
	}

	line->attenuation.us = 0;
	line->attenuation.ds = 48;

	line->power.us = 123;
	line->power.ds = 106;

	// XTU-R vendor ID
	strncpy(line->xtur_vendor, "B5004244434D0000", sizeof(line->xtur_vendor));

	// XTU-C vendor ID
	strncpy(line->xtuc_vendor, "B5004244434DA491", sizeof(line->xtuc_vendor));

	// XTC-C vendor revision number
	line->xtuc_ansi_rev = 0xa491;

	/*
	 * The following parameters are not supported on Broadcom platform. Dummy values are assigned
	 * as per the meaning of the parameters accordingly.
	 */
	line->success_failure_cause = 0;
	line->act_ra_mode.us = line->act_ra_mode.ds = 0; // Actual active rate adaptation mode
	line->line_number = 0;
	// Actual signal-to-noise margin of the robust overhead channel (ROC) in the upstream direction
	line->snr_mroc.us = 0;
	strncpy(line->xtur_country, "0000", sizeof(line->xtur_country)); // XTU-R country
	line->xtur_ansi_std = line->xtur_ansi_rev = 0; // XTU-R revision number and vendor revision number
	strncpy(line->xtuc_country, "0000", sizeof(line->xtuc_country)); // XTU-C country
	line->xtuc_ansi_std = 0; // XTU-C revision number

	return 0;
}


int dsl_get_line_stats(int line_num, struct dsl_line_channel_stats *stats)
{
	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Fill in the values
	 */
	stats->total_start = 397;
	stats->showtime_start = 349;
	stats->last_showtime_start = 349;
	stats->current_day_start = 397;
	stats->quarter_hour_start = 397;

	return 0;
}

int dsl_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct dsl_line_stats_interval *stats)
{
	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Convert the values
	 */
	switch (type) {
	case DSL_STATS_TOTAL:
		stats->errored_secs = 0;
		stats->severely_errored_secs = 0;
		break;
	case DSL_STATS_SHOWTIME:
		stats->errored_secs = 0;
		stats->severely_errored_secs = 0;
		break;
	case DSL_STATS_LASTSHOWTIME: // N/A
		stats->errored_secs = DSL_INVALID_STATS_COUNTER;
		stats->severely_errored_secs = DSL_INVALID_STATS_COUNTER;
		break;
	case DSL_STATS_CURRENTDAY:
		stats->errored_secs = 0;
		stats->severely_errored_secs = 0;
		break;
	case DSL_STATS_QUARTERHOUR:
		stats->errored_secs = 0;
		stats->severely_errored_secs = 0;
		break;
	default:
		LIBDSL_LOG(LOG_ERR, "Unknown interval type for DSL line statistics, %d\n", type);
		return -1;
	}

	return 0;
}

int dsl_get_channel_info(int chan_num, struct dsl_channel *channel)
{
	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Initialize the output buffer
	memset(channel, 0, sizeof(*channel));

	/**
	 *  Fill in the values
	 */
	channel->status = IF_UP;

	// Link encapsulation supported and currently used
	channel->link_encapsulation_supported = G_992_3_ANNEK_K_ATM | G_992_3_ANNEK_K_PTM |
			G_993_2_ANNEK_K_ATM | G_993_2_ANNEK_K_PTM | G_994_1_AUTO;
	channel->link_encapsulation_used = G_993_2_ANNEK_K_PTM;

	// Latency path information
	channel->lpath = 0;
	channel->intlvdepth = 1;
	channel->intlvblock = 120;
	channel->actual_interleaving_delay = 0;
	channel->actinp = 0;

	// Reed-Solomon (RS) codeword size
	channel->nfec = 240;
	channel->lsymb = 15080;

	// Physical layer aggregate data rate
	channel->curr_rate = (dsl_ulong_t){60085,100120};

	// Actual net data rate
	channel->actndr = (dsl_ulong_t){59999, 99976};

	// Actual impulse noise protection again REIN
	channel->actinprein = (dsl_ulong_t){0, 0};

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
	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/**
	 *  Fill in the values
	 */
	stats->current_day_start = 397;
	stats->quarter_hour_start = 397;

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
	if (chan_num < 0 || chan_num >= max_chan_num)
		return -1;

	// Initialize the output buffer
	stats->xtur_fec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_fec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtur_hec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_hec_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtur_crc_errors = DSL_INVALID_STATS_COUNTER;
	stats->xtuc_crc_errors = DSL_INVALID_STATS_COUNTER;

	/**
	 *  Fill in the values
	 */
	switch (type) {
	case DSL_STATS_TOTAL:
		stats->xtur_fec_errors = 0;
		stats->xtuc_fec_errors = 0;
		stats->xtur_crc_errors = 0;
		stats->xtuc_crc_errors = 0;
		break;
	case DSL_STATS_SHOWTIME:
		stats->xtur_fec_errors = 0;
		stats->xtuc_fec_errors = 0;
		/*
		 * HEC counters actually are not included in any periodical statistics. But driver source shows that
		 * it is in the category of showtime. And it also complies to the test results by Telia Eesti in
		 * ticket #1043.
		 */
		stats->xtur_hec_errors = 0;
		stats->xtuc_hec_errors = 0;
		stats->xtur_crc_errors = 0;
		stats->xtuc_crc_errors = 0;
		break;
	case DSL_STATS_LASTSHOWTIME: // N/A
		break;
	case DSL_STATS_CURRENTDAY:
		stats->xtur_fec_errors = 0;
		stats->xtuc_fec_errors = 0;
		stats->xtur_crc_errors = 0;
		stats->xtuc_crc_errors = 0;
		break;
	case DSL_STATS_QUARTERHOUR:
		stats->xtur_fec_errors = 0;
		stats->xtuc_fec_errors = 0;
		stats->xtur_crc_errors = 0;
		stats->xtuc_crc_errors = 0;
		break;
	default:
		LIBDSL_LOG(LOG_ERR, "Unknown interval type for DSL channel statistics, %d\n", type);
		return -1;
	}

	return 0;
}

int dsl_get_line_test_params(int line_num, struct dsl_line_test_params *test_params)
{
	unsigned long elements[4] = { 1, 2, 3, 4 };

	if (line_num < 0 || line_num >= max_line_num || !test_params)
		return -1;

	// Initialize the output buffer
	memset(test_params, 0, sizeof(*test_params));

	/*
	 *  Fill in fake values
	 */
	test_params->hlogg.ds = 1;
	test_params->hlogg.us = 2;
	dsl_alloc_ulong_elements(&test_params->hlogps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->hlogps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->hlogps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->hlogps_us.elements, elements, sizeof(elements));
	test_params->hlogmt.ds = 1;
	test_params->hlogmt.us = 2;

	test_params->qlng.ds = 3;
	test_params->qlng.us = 4;
	dsl_alloc_ulong_elements(&test_params->qlnps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->qlnps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->qlnps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->qlnps_us.elements, elements, sizeof(elements));
	test_params->qlnmt.ds = 3;
	test_params->qlnmt.us = 4;

	test_params->snrg.ds = 1;
	test_params->snrg.us = 2;
	dsl_alloc_ulong_elements(&test_params->snrps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->snrps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->snrps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->snrps_us.elements, elements, sizeof(elements));
	test_params->snrmt.ds = 1;
	test_params->snrmt.us = 2;

	dsl_alloc_ulong_elements(&test_params->latn_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->latn_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->latn_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->latn_us.elements, elements, sizeof(elements));

	dsl_alloc_ulong_elements(&test_params->satn_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->snrps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->satn_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->satn_us.elements, elements, sizeof(elements));

	return 0;
}

int dsl_get_line_data_gathering(int line_num, struct dsl_line_data_gathering *data_gathering)
{
	if (line_num < 0 || line_num >= max_line_num || !data_gathering)
		return -1;

	// Initialize the output buffer
	memset(data_gathering, 0, sizeof(*data_gathering));

	/*
	 *  Fill in fake values
	 */
	data_gathering->logging_depth_r = 64;
	data_gathering->act_logging_depth_r = 32;

	return 0;
}

int adsl_line_test(int line_num, bool start)
{
	if (line_num < 0 || line_num >= max_line_num)
		return -1;

	return 0;
}

enum diag_state adsl_get_line_test_results(int line_num, struct adsl_line_test *results)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };
	unsigned long array[3] = { 2, 4, 6 };

	if (line_num < 0 || line_num >= max_line_num || !results)
		return -1;

	// Initialize the output buffer
	memset(results, 0, sizeof(*results));

	/*
	 *  Fill in fake values
	 */
	results->act_psd.ds = 1;
	results->act_psd.us = 2;
	results->act_atp.ds = 3;
	results->act_atp.us = 4;

	results->hlinsc.ds = -1;
	results->hlinsc.us = -2;
	results->hling.ds = 3;
	results->hling.us = 4;

	results->hlogg.ds = 1;
	results->hlogg.us = 2;
	dsl_alloc_ulong_elements(&results->hlogps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->hlogps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&results->hlogps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->hlogps_us.elements, elements, sizeof(elements));
	results->hlogmt.ds = 3;
	results->hlogmt.us = 4;

	results->latnpb_ds.count = sizeof(array) / sizeof(array[0]);
	memcpy(results->latnpb_ds.array, array, sizeof(array));
	results->latnpb_us.count = sizeof(array) / sizeof(array[0]);
	memcpy(results->latnpb_us.array, array, sizeof(array));

	results->satn_ds.count = sizeof(array) / sizeof(array[0]);
	memcpy(results->satn_ds.array, array, sizeof(array));
	results->satn_us.count = sizeof(array) / sizeof(array[0]);
	memcpy(results->satn_us.array, array, sizeof(array));

	dsl_alloc_ulong_elements(&results->hlinps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->hlinps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&results->hlinps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->hlinps_us.elements, elements, sizeof(elements));

	results->qlng.ds = 1;
	results->qlng.us = 2;
	dsl_alloc_ulong_elements(&results->qlnps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->qlnps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&results->qlnps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->qlnps_us.elements, elements, sizeof(elements));

	results->qlnmt.ds = 1;
	results->qlnmt.us = 2;

	results->snrg.ds = 1;
	results->snrg.us = 2;
	dsl_alloc_ulong_elements(&results->snrps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->snrps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&results->snrps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->snrps_us.elements, elements, sizeof(elements));

	results->snrmt.ds = 1;
	results->snrmt.us = 2;

	dsl_alloc_ulong_elements(&results->bitsps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->bitsps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&results->bitsps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(results->bitsps_us.elements, elements, sizeof(elements));

	return ATM_DIAG_COMPLETE;
}

int dsl_seltuer_test(int line_num, const struct dsl_seltuer *seltuer, bool start)
{
	if (line_num < 0 || line_num >= max_line_num || !seltuer)
		return -1;

	return 0;
}

enum diag_state dsl_get_seltuer_results(int line_num, struct dsl_seltuer *seltuer)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };

	if (line_num < 0 || line_num >= max_line_num || !seltuer)
		return -1;

	// Initialize the output buffer
	memset(seltuer, 0, sizeof(*seltuer));

	/*
	 *  Fill in fake values
	 */
	seltuer->uer_max_duration = 120;
	seltuer->ext_bandwidth_op = true;

	dsl_alloc_ulong_elements(&seltuer->uer, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltuer->uer.elements, elements, sizeof(elements));

	seltuer->uer_scale_factor = 8;
	seltuer->uer_group_size = 1;

	dsl_alloc_ulong_elements(&seltuer->uer_var, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltuer->uer_var.elements, elements, sizeof(elements));

	return ATM_DIAG_COMPLETE;
}

int dsl_seltqln_test(int line_num, const struct dsl_seltqln *seltqln, bool start)
{
	if (line_num < 0 || line_num >= max_line_num || !seltqln)
		return -1;

	return 0;
}

enum diag_state dsl_get_seltqln_results(int line_num, struct dsl_seltqln *seltqln)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };

	if (line_num < 0 || line_num >= max_line_num || !seltqln)
		return -1;

	// Initialize the output buffer
	memset(seltqln, 0, sizeof(*seltqln));

	/*
	 *  Fill in fake values
	 */
	seltqln->qln_max_measurement_duration = 180;
	seltqln->ext_bandwidth_op = true;

	dsl_alloc_ulong_elements(&seltqln->qln, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltqln->qln.elements, elements, sizeof(elements));

	seltqln->qln_group_size = 1;

	return ATM_DIAG_COMPLETE;
}

int dsl_seltp_test(int line_num, const struct dsl_seltp *seltp, bool start)
{
	if (line_num < 0 || line_num >= max_line_num || !seltp)
		return -1;

	return 0;
}

enum diag_state dsl_get_seltp_results(int line_num, struct dsl_seltp *seltp)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };
	int i;

	if (line_num < 0 || line_num >= max_line_num || !seltp)
		return -1;

	// Initialize the output buffer
	memset(seltp, 0, sizeof(*seltp));

	/*
	 *  Fill in fake values
	 */
	seltp->capacity_estimate_enable = true;

	dsl_alloc_ulong_elements(&seltp->capacity_signal_psd, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltp->capacity_signal_psd.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&seltp->capacity_noise_psd, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltp->capacity_noise_psd.elements, elements, sizeof(elements));

	seltp->capacity_target_margin = 16;
	seltp->loop_term = POWERED_ON;
	seltp->loop_length = 8;

	seltp->loop_topology.count = 4;
	for (i = 0; i < seltp->loop_topology.count; i++) {
		seltp->loop_topology.pairs[i].loop_seg_len = i + 1;
		seltp->loop_topology.pairs[i].loop_seg_btap = 2 * (i + 1);
	}

	dsl_alloc_ulong_elements(&seltp->attenuation_characteristics, sizeof(elements) / sizeof(elements[0]));
	memcpy(seltp->attenuation_characteristics.elements, elements, sizeof(elements));

	seltp->missing_filter = true;
	seltp->capacity_estimate = 12;

	return ATM_DIAG_COMPLETE;
}

int fast_get_line_info(int line_num, struct fast_line *line)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };

	if (line_num < 0 || line_num >= max_line_num || !line)
		return -1;

	// Initialize the output buffer
	memset(line, 0, sizeof(*line));

	/*
	 *  Fill in fake values
	 */
	line->status = IF_UP;
	line->upstream = true;
	strncpy(line->firmware_version, "A2pvfbH045k.d28a", sizeof(line->firmware_version) - 1);
	line->link_status = LINK_UP;

	line->allowed_profiles = FAST_106a | FAST_212a;
	line->current_profile = FAST_106a;

	line->power_management_state = FAST_L0;
	line->success_failure_cause = 0;

	line->upbokler = 180;
	line->last_transmitted_signal.us = 100;
	line->last_transmitted_signal.ds = 200;
	line->upbokle = 160;
	line->line_number = 1;

	if (line->status == IF_UP) {
		line->max_bit_rate.us = 201000;
		line->max_bit_rate.ds = 402000;

		line->noise_margin.us = 146;
		line->noise_margin.ds = 177;
	}

	line->attenuation.us = 0;
	line->attenuation.ds = 48;

	line->power.us = 123;
	line->power.ds = 106;

	line->snrm_rmc.us = 12;
	line->snrm_rmc.ds = 14;

	dsl_alloc_ulong_elements(&line->bits_rmc_ps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(line->bits_rmc_ps_ds.elements, elements, sizeof(elements));

	dsl_alloc_ulong_elements(&line->bits_rmc_ps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(line->bits_rmc_ps_us.elements, elements, sizeof(elements));

	line->fext_to_cancel_enable.us = true;
	line->fext_to_cancel_enable.ds = true;

	line->etr.us = 250000;
	line->etr.ds = 350000;
	line->att_etr.us = 225000;
	line->att_etr.ds = 325000;
	line->min_eftr.us = 200000;
	line->min_eftr.ds = 300000;

	return 0;
}

int fast_get_line_stats(int line_num, struct fast_line_stats *stats)
{
	if (line_num < 0 || line_num >= max_line_num || !stats)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/*
	 * Fill in fake values
	 */
	stats->total_start = 397;
	stats->showtime_start = 349;
	stats->last_showtime_start = 349;
	stats->current_day_start = 397;
	stats->quarter_hour_start = 397;

	return 0;
}

int fast_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct fast_line_stats_interval *stats)
{
	if (line_num < 0 || line_num >= max_line_num || !stats)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/*
	 *  Fill in fake values
	 */
	stats->errored_secs = 20;
	stats->severely_errored_secs = 5;

	stats->loss = 1;
	stats->lors = DSL_INVALID_STATS_COUNTER;
	stats->uas = 1;
	stats->rtx_uc = DSL_INVALID_STATS_COUNTER;
	stats->rtx_tx = 0;
	stats->success_bsw = 1024;
	stats->success_sra = 2048;
	stats->success_fra = 1024;
	stats->success_rpa = 2048;
	stats->success_tiga = 1024;

	return 0;
}

int fast_get_line_test_params(int line_num, struct fast_line_test_params *test_params)
{
	unsigned long elements[4] = { 1, 3, 5, 7 };

	if (line_num < 0 || line_num >= max_line_num || !test_params)
		return -1;

	// Initialize the output buffer
	memset(test_params, 0, sizeof(*test_params));

	/*
	 *  Fill in fake values
	 */
	test_params->snrg = (dsl_ulong_t){ 1, 1 };

	dsl_alloc_ulong_elements(&test_params->snrps_ds, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->snrps_ds.elements, elements, sizeof(elements));
	dsl_alloc_ulong_elements(&test_params->snrps_us, sizeof(elements) / sizeof(elements[0]));
	memcpy(test_params->snrps_us.elements, elements, sizeof(elements));

	test_params->snrmt = (dsl_ulong_t){ 128, 192 };

	test_params->act_inp = 4;
	test_params->nfec = 2;
	test_params->rfec = 0;

	test_params->curr_rate = (dsl_ulong_t){ 251000, 423000 };
	test_params->act_inp_rein = 5;

	return 0;
}

int dsl_configure(int line_num, struct dsl_config_params *cfg_params)
{
	int rc = 0;
	unsigned char xtse_invalid[8] = { 0, };
	char input_params[512];
	char xtse_str[2 * 8 + 7 + 1] = "";
	char str_vdsl2_profiles[160], str_fast_profiles[40];
	int profile, total_len, len;

	if (line_num < 0 || line_num >= max_line_num || !cfg_params)
		return -1;

	// The input parameters validation check below is indicative only and shall not be treated literally.
	// The actual validation check depends upon the particular hardware platform's capacity.
	if (memcmp(cfg_params->xtse, xtse_invalid, sizeof(xtse_invalid)) == 0 ||
		(cfg_params->vdsl2_profiles == 0 && cfg_params->fast_profiles == 0)) {
		rc = -1;
	}

	// xtse
	snprintf(xtse_str, sizeof(xtse_str), "%02hhx,%02hhx,%02hhx,%02hhx,%02hhx,%02hhx,%02hhx,%02hhx",
			cfg_params->xtse[0],cfg_params->xtse[1],cfg_params->xtse[2],cfg_params->xtse[3],
			cfg_params->xtse[4],cfg_params->xtse[5],cfg_params->xtse[6],cfg_params->xtse[7]);

	// vdsl2_profiles
	for (profile = VDSL2_8a, total_len = 0; profile <= VDSL2_35b; profile <<= 1) {
		if (cfg_params->vdsl2_profiles & profile) {
			const char *str = dsl_get_string_value(vdsl2_profiles, profile);
			if (str && *str) {
				len = strlen(str);
				if (total_len == 0) {
					strncpy(str_vdsl2_profiles, str, sizeof(str_vdsl2_profiles) - 1);
					total_len += len;
				} else if (total_len + len + 1 < sizeof(str_vdsl2_profiles)) {
					snprintf(str_vdsl2_profiles + total_len, sizeof(str_vdsl2_profiles), ",%s", str);
					total_len += (len + 1);
				}
			}
		}
	}

	// fast_profiles
	for (profile = FAST_106a, total_len = 0; profile <= FAST_212a; profile <<= 1) {
		if (cfg_params->fast_profiles & profile) {
			const char *str = dsl_get_string_value(fast_profiles, profile);
			if (str && *str) {
				len = strlen(str);
				if (total_len == 0) {
					strncpy(str_fast_profiles, str, sizeof(str_fast_profiles) - 1);
					total_len += len;
				} else if (total_len + len + 1 < sizeof(str_fast_profiles)) {
					snprintf(str_fast_profiles + total_len, sizeof(str_fast_profiles), ",%s", str);
					total_len += (len + 1);
				}
			}
		}
	}

	snprintf(input_params, sizeof(input_params), "{'rc':%d,'xtse':'%s','vdsl2_profiles':'%s',"
	    "'fast_profiles':'%s','data_gathering':%s,'limit_mask':%u,'us0_mask':%u}", rc,
		xtse_str, str_vdsl2_profiles, str_fast_profiles, cfg_params->enable_data_gathering ? "true" : "false",
		cfg_params->limit_mask, cfg_params->us0_mask);
	log_test("%s(): returned value and input parameters dump: %s\n", __func__, input_params);

	return rc;
}

/* ======================================= ATM ===================================================== */
const struct atm_ops atm_funcs = {
		.configure = atm_configure,
		.get_link_info = atm_get_link_info,
		.get_link_stats = atm_get_link_stats,
		.loopback_test = atm_loopback_test,
		.get_loopback_results = atm_get_loopback_results,
};

int atm_configure(int link_num, const struct atm_link *link, const struct atm_link_qos *qos)
{
	int rc = 0;
	char input_params[512];

	if (link_num >= max_chan_num || !link || !qos)
		return -1;

	// The input parameters validation check below is indicative only and shall not be treated literally.
	// The actual validation check depends upon the particular hardware platform's capacity.
	if (link->link_type == ATM_LINK_UNCONFIGURED ||
		link->dest_addr.vpi == 0 || link->dest_addr.vci == 0 ||
		qos->qos_class == 0) {
		rc = -1;
	}

	const char *str_link_type = dsl_get_string_value(atm_link_types, link->link_type);
	const char *str_encap = dsl_get_string_value(atm_encapsulations, link->encapsulation);
	const char *str_qos_class = dsl_get_string_value(atm_qos_classes, qos->qos_class);
	snprintf(input_params, sizeof(input_params), "{'rc':%d,'link_type':'%s','vpi':%u,'vci':%u,'encapsulation':'%s',"
		    "'qos_class':'%s','peak_cell_rate':%u,'max_burst_size':%u,'sustainable_cell_rate':%u}", rc,
			str_link_type ? str_link_type : "", link->dest_addr.vpi, link->dest_addr.vci,
			str_encap ? str_encap : "", str_qos_class ? str_qos_class : "",
			qos->peak_cell_rate, qos->max_burst_size, qos->sustainable_cell_rate);
	log_test("%s(): returned value and input parameters dump: %s\n", __func__, input_params);

	return rc;
}

int atm_get_link_info(int link_num, struct atm_link *link, struct atm_link_qos *qos)
{
	if (link_num >= max_chan_num || !link || !qos)
		return -1;

	// Initialize the output buffer
	memset(link, 0, sizeof(*link));
	memset(qos, 0, sizeof(*qos));

	/*
	 *  Fill in fake values
	 */
	// Link info
	link->status = IF_UP;
	link->link_type = ATM_LINK_EoA;
	link->auto_config = true;
	link->dest_addr.vpi = 8;
	link->dest_addr.vci = 35;
	link->encapsulation = ATM_LLC;
	link->fcs_preserved = true;

	link->vc_list_count = 2;
	link->vc_search_list = malloc(sizeof(struct atm_dest_addr) * link->vc_list_count);
	if (!link->vc_search_list)
		return -1;
	struct atm_dest_addr addrs[2] = { { 8, 36 }, { 8, 37 } };
	memcpy(link->vc_search_list, addrs, sizeof(struct atm_dest_addr) * link->vc_list_count);

	link->aal = ATM_AAL5;

	// QoS
	qos->qos_class = ATM_QoS_UBR;
	qos->peak_cell_rate = 18867; //  8000000/(53*8)
	qos->max_burst_size = 22640; // peak_cell_rate*1.2
	qos->sustainable_cell_rate = 15093; // peak_cell_rate*0.8

	return 0;
}

int atm_get_link_stats(int link_num, struct atm_link_stats *stats)
{
	if (link_num >= max_chan_num || !stats)
		return -1;

	// Initialize the output buffer
	memset(stats, 0, sizeof(*stats));

	/*
	 *  Fill in fake values
	 */
	stats->transmitted_blocks = 12345678;
	stats->received_blocks = 87654321;
	stats->crc_errors = 12;

	return 0;
}

int atm_loopback_test(int link_num, const struct atm_diag_loopback *loopback, bool start)
{
	if (link_num >= max_chan_num || !loopback)
		return -1;

	// Does nothing for now
	// TODO: to be implemented in the next stage

	return 0;
}

enum diag_state atm_get_loopback_results(int link_num, struct atm_diag_loopback *loopback)
{
	if (link_num >= max_chan_num || !loopback)
		return ATM_DIAG_ERROR_OTHER;

	// Initialize the output buffer
	memset(loopback, 0, sizeof(*loopback));

	/*
	 *  Fill in fake values
	 */
	// TODO: to be implemented in the next stage

	return ATM_DIAG_COMPLETE;
}

/* ======================================= PTM ===================================================== */
const struct ptm_ops ptm_funcs = {
		.get_link_info = ptm_get_link_info,
};

int ptm_get_link_info(int link_num, struct ptm_link *link)
{
	if (link_num >= max_chan_num || !link)
		return -1;

	// Initialize the output buffer
	memset(link, 0, sizeof(*link));

	/*
	 *  Fill in fake values
	 */
	link->status = IF_UP;
	unsigned char addr[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
	memcpy(link->mac_addr, addr, sizeof(link->mac_addr));

	return 0;
}
