/*
 * libdsl_test.c - libdsl test application
 *
 * Copyright (C) 2019 iopsys Software Solutions AB. All rights reserved.
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
#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "common.h"
#include "xdsl.h"
#include "xtm.h"

static const char *dsl_get_stats_interval_type(enum dsl_stats_type type)
{
	static const struct {
		enum dsl_stats_type type;
		char *text;
	} stats_types[] = {
		{ DSL_STATS_TOTAL, "Total" },
		{ DSL_STATS_SHOWTIME, "Showtime" },
		{ DSL_STATS_LASTSHOWTIME, "LastShowtime" },
		{ DSL_STATS_CURRENTDAY, "CurrentDay" },
		{ DSL_STATS_QUARTERHOUR, "QuarterHour" }
	};
	int i;

	for (i = 0; i < sizeof(stats_types) / sizeof(stats_types[0]); i++) {
		if (type == stats_types[i].type)
			return stats_types[i].text;
	}

	return "Unknown";
}

int main(int argc, char *argv[])
{
	int max_line, max_chan;
	struct dsl_line line;
	struct dsl_line_channel_stats stats;
	enum dsl_stats_type type;
	struct dsl_line_stats_interval stats_interval;
	struct dsl_channel channel;
	struct dsl_channel_stats_interval chan_stats_interval;
	int i, j;

	// Get numbers of DSL lines and channels
	max_line = dsl_get_line_number();
	max_chan = dsl_get_channel_number();

	for (j = 0; j < max_line; j++) {
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		printf("Line.%d\n", j);
		// Line
		if (dsl_get_line_info(j, &line) == 0) {
			printf("------------------------------------------------------------\n");
			printf("Line: status = %d\n", line.status);
			printf("Line: upstream = %s\n", line.upstream ? "true" : "false");
			printf("Line: firmware_version = %s\n", line.firmware_version);
			printf("Line: link_status = %d\n", line.link_status);
			if (line.standard_supported.use_xtse) {
				printf("Line: xtse = {0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}\n",
					line.standard_supported.xtse[0], line.standard_supported.xtse[1],
					line.standard_supported.xtse[2], line.standard_supported.xtse[3],
					line.standard_supported.xtse[4], line.standard_supported.xtse[5],
					line.standard_supported.xtse[6], line.standard_supported.xtse[7]);
				printf("Line: standard_supported\n");
				for (i = T1_413; i <= G_993_2_JAPAN; i++) {
					if (XTSE_BIT_GET(line.standard_supported.xtse, i))
						printf("  Bit %d is included\n", i);
				}
			} else {
				printf("Line: standard_supported = 0x%lx\n", line.standard_supported.mode);
			}
			if (line.standard_used.use_xtse) {
				printf("Line: xtse_used = {0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}\n",
					line.standard_used.xtse[0], line.standard_used.xtse[1],
					line.standard_used.xtse[2], line.standard_used.xtse[3],
					line.standard_used.xtse[4], line.standard_used.xtse[5],
					line.standard_used.xtse[6], line.standard_used.xtse[7]);
				printf("Line: standard_used\n");
				for (i = T1_413; i <= G_993_2_JAPAN; i++) {
					if (XTSE_BIT_GET(line.standard_used.xtse, i))
						printf("  Bit %d is included\n", i);
				}
			} else {
				printf("Line: standard_used = 0x%lx\n", line.standard_used.mode);
			}
			printf("Line: line_encoding = %d\n", line.line_encoding);
			printf("Line: allowed_profiles = 0x%lx\n", line.allowed_profiles);
			printf("Line: current_profile = 0x%x\n", line.current_profile);

			printf("Line: power_management_state = %d\n", line.power_management_state);
			printf("Line: success_failure_cause = %u\n", line.success_failure_cause);

			printf("Line: upbokler_pb = {");
			for (i = 0; i < line.upbokler_pb.count; i++)
				printf("%s%lu", i > 0 ? "," : "", line.upbokler_pb.array[i]);
			printf("}\n");

			printf("Line: rxthrsh_ds = {");
			for (i = 0; i < line.rxthrsh_ds.count; i++)
				printf("%s%lu", i > 0 ? "," : "", line.rxthrsh_ds.array[i]);
			printf("}\n");

			printf("Line: act_ra_mode = {%lu,%lu}\n", line.act_ra_mode.us, line.act_ra_mode.ds);
			printf("Line: snr_mroc.us = %u\n", line.snr_mroc.us);
			printf("Line: last_state_transmitted = {%lu,%lu}\n",
					line.last_state_transmitted.us, line.last_state_transmitted.ds);
			printf("Line: us0_mask = 0x%x\n", line.us0_mask);
			printf("Line: trellis = {%ld,%ld}\n", line.trellis.us, line.trellis.ds);
			printf("Line: act_snr_mode = {%lu,%lu}\n", line.act_snr_mode.us, line.act_snr_mode.ds);
			printf("Line: line_number = %d\n", line.line_number);
			printf("Line: max_bit_rate = {%lukbps,%lukbps}\n", line.max_bit_rate.us, line.max_bit_rate.ds);
			printf("Line: noise_margin = {%ld,%ld}\n", line.noise_margin.us, line.noise_margin.ds);

			printf("Line: snr_mpb_us = {");
			for (i = 0; i < line.snr_mpb_us.count; i++)
				printf("%s%ld", i > 0 ? "," : "", line.snr_mpb_us.array[i]);
			printf("}\n");

			printf("Line: snr_mpb_ds = {");
			for (i = 0; i < line.snr_mpb_ds.count; i++)
				printf("%s%ld", i > 0 ? "," : "", line.snr_mpb_ds.array[i]);
			printf("}\n");

			printf("Line: attenuation = {%ld,%ld}\n", line.attenuation.us, line.attenuation.ds);
			printf("Line: power = {%ld,%ld}\n", line.power.us, line.power.ds);

			printf("Line: xtur_vendor = %s\n", line.xtur_vendor);
			printf("Line: xtur_country = %s\n", line.xtur_country);
			printf("Line: xtur_ansi_std = 0x%x\n", line.xtur_ansi_std);
			printf("Line: xtur_ansi_rev = 0x%x\n", line.xtur_ansi_rev);

			printf("Line: xtuc_vendor = %s\n", line.xtuc_vendor);
			printf("Line: xtuc_country = %s\n", line.xtuc_country);
			printf("Line: xtuc_ansi_std = 0x%x\n", line.xtuc_ansi_std);
			printf("Line: xtuc_ansi_rev = 0x%x\n", line.xtuc_ansi_rev);
		}

		// Line statistics
		if (xdsl_ops.get_line_stats != NULL && (*xdsl_ops.get_line_stats)(j, &stats) == 0) {
			printf("------------------------------------------------------------\n");
			printf("Line.Stats: total_start = %u\n", stats.total_start);
			printf("Line.Stats: showtime_start = %u\n", stats.showtime_start);
			printf("Line.Stats: last_showtime_start = %u\n", stats.last_showtime_start);
			printf("Line.Stats: current_day_start = %u\n", stats.current_day_start);
			printf("Line.Stats: quarter_hour_start = %u\n", stats.quarter_hour_start);
		}

		// Line interval statistics
		for (type = DSL_STATS_TOTAL; type <= DSL_STATS_QUARTERHOUR; type++) {
			if (xdsl_ops.get_line_stats_interval != NULL &&
				(*xdsl_ops.get_line_stats_interval)(j, type, &stats_interval) == 0) {
				printf("------------------------------------------------------------\n");
				printf("Line.Stats.%s: errored_secs = %u\n", dsl_get_stats_interval_type(type),
						stats_interval.errored_secs);
				printf("Line.Stats.%s: severely_errored_secs = %u\n", dsl_get_stats_interval_type(type),
						stats_interval.severely_errored_secs);
			}
		}
	}

	for (j = 0; j < max_chan; j++) {
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		printf("Channel.%d\n", j);
		// Channel
		if (xdsl_ops.get_channel_info != NULL && (*xdsl_ops.get_channel_info)(j, &channel) == 0) {
			printf("------------------------------------------------------------\n");
			printf("Channel: status = %d\n", channel.status);
			printf("Channel: link_encapsulation_supported = 0x%lx\n", channel.link_encapsulation_supported);
			printf("Channel: link_encapsulation_used = 0x%x\n", channel.link_encapsulation_used);
			printf("Channel: lpath = %u\n", channel.lpath);
			printf("Channel: intlvdepth = %u\n", channel.intlvdepth);
			printf("Channel: intlvblock = %u\n", channel.intlvblock);
			printf("Channel: actual_interleaving_delay = %u\n", channel.actual_interleaving_delay);
			printf("Channel: actinp = %d\n", channel.actinp);
			printf("Channel: inpreport = %s\n", channel.inpreport ? "true" : "false");
			printf("Channel: nfec = %d\n", channel.nfec);
			printf("Channel: rfec = %d\n", channel.rfec);
			printf("Channel: lsymb = %d\n", channel.lsymb);
			printf("Channel: curr_rate = {%lukbps,%lukpbs}\n", channel.curr_rate.us, channel.curr_rate.ds);
			printf("Channel: actndr = {%lukbps,%lukpbs}\n", channel.actndr.us, channel.actndr.ds);
			printf("Channel: actinprein = {%lu,%lu}\n", channel.actinprein.us, channel.actinprein.ds);
		}

		// Channel statistics
		if (xdsl_ops.get_channel_stats != NULL && (*xdsl_ops.get_channel_stats)(j, &stats) == 0) {
			printf("------------------------------------------------------------\n");
			printf("Channel.Stats: total_start = %u\n", stats.total_start);
			printf("Channel.Stats: showtime_start = %u\n", stats.showtime_start);
			printf("Channel.Stats: last_showtime_start = %u\n", stats.last_showtime_start);
			printf("Channel.Stats: current_day_start = %u\n", stats.current_day_start);
			printf("Channel.Stats: quarter_hour_start = %u\n", stats.quarter_hour_start);
		}

		// Channel interval statistics
		for (type = DSL_STATS_TOTAL; type <= DSL_STATS_QUARTERHOUR; type++) {
			if (xdsl_ops.get_channel_stats_interval != NULL &&
				(*xdsl_ops.get_channel_stats_interval)(j, type, &chan_stats_interval) == 0) {
				printf("------------------------------------------------------------\n");
				printf("Channel.Stats.%s: xtur_fec_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtur_fec_errors);
				printf("Channel.Stats.%s: xtuc_fec_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtuc_fec_errors);

				printf("Channel.Stats.%s: xtur_hec_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtur_hec_errors);
				printf("Channel.Stats.%s: xtuc_hec_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtuc_hec_errors);

				printf("Channel.Stats.%s: xtur_crc_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtur_crc_errors);
				printf("Channel.Stats.%s: xtuc_crc_errors = %u\n", dsl_get_stats_interval_type(type),
						chan_stats_interval.xtuc_crc_errors);
			}
		}
	}

	return 0;
}
