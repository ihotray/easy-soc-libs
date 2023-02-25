/*
 * xdsl.h - library header file
 * This file provides definition for the libdsl APIs and related
 * structures.
 *
 * Copyright (C) 2020 iopsys Software Solutions AB. All rights reserved.
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
#ifndef _XDSL_H
#define _XDSL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

/** Common definitions */
#define XDSL_MAX_LINES	1

typedef struct { long us; long ds; } dsl_long_t;
typedef struct { unsigned long us; unsigned long ds; } dsl_ulong_t;
typedef struct { bool us; bool ds; } dsl_bool_t;
typedef struct { long array[24]; int count; } dsl_long_sequence_t;
typedef struct { unsigned long array[24]; int count; } dsl_ulong_sequence_t;
typedef struct { unsigned long *elements; int count; } dsl_ulong_elements_t;

/** enum dsl_link_status - link status */
enum dsl_link_status {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	LINK_UP = 1,
	LINK_INITIALIZING,
	LINK_ESTABLISHING,
	LINK_NOSIGNAL,
	LINK_DISABLED,
	LINK_ERROR
};

/** enum dsl_modtype - DSL modes */
enum dsl_modtype {
	MOD_G_922_1_ANNEX_A = 1,
	MOD_G_922_1_ANNEX_B = 1 << 1,
	MOD_G_922_1_ANNEX_C = 1 << 2,
	MOD_T1_413          = 1 << 3,
	MOD_T1_413i2        = 1 << 4,
	MOD_ETSI_101_388    = 1 << 5,
	MOD_G_992_2         = 1 << 6,
	MOD_G_992_3_Annex_A = 1 << 7,
	MOD_G_992_3_Annex_B = 1 << 8,
	MOD_G_992_3_Annex_C = 1 << 9,
	MOD_G_992_3_Annex_I = 1 << 10,
	MOD_G_992_3_Annex_J = 1 << 11,
	MOD_G_992_3_Annex_L = 1 << 12,
	MOD_G_992_3_Annex_M = 1 << 13,
	MOD_G_992_4         = 1 << 14,
	MOD_G_992_5_Annex_A = 1 << 15,
	MOD_G_992_5_Annex_B = 1 << 16,
	MOD_G_992_5_Annex_C = 1 << 17,
	MOD_G_992_5_Annex_I = 1 << 18,
	MOD_G_992_5_Annex_J = 1 << 19,
	MOD_G_992_5_Annex_M = 1 << 20,
	MOD_G_993_1         = 1 << 21,
	MOD_G_993_1_Annex_A = 1 << 22,
	MOD_G_993_2_Annex_A = 1 << 23,
	MOD_G_993_2_Annex_B = 1 << 24,
	MOD_G_993_2_Annex_C = 1 << 25
};

/** enum dsl_xtse_bit - XTSE bit definition. Refer to dsl_line.xtse for details */
enum dsl_xtse_bit {
	/* Octet 1 - ADSL, i.e. g.dmt */
	T1_413							= 1,
	ETSI_101_388					= 2,
	G_992_1_POTS_NON_OVERLAPPED		= 3, /* Annex A */
	G_992_1_POTS_OVERLAPPED			= 4, /* Annex A */
	G_992_1_ISDN_NON_OVERLAPPED		= 5, /* Annex B */
	G_992_1_ISDN_OVERLAPPED			= 6, /* Annex B */
	G_992_1_TCM_ISDN_NON_OVERLAPPED	= 7, /* Annex C */
	G_992_1_TCM_ISDN_OVERLAPPED		= 8, /* Annex C */

	/* Octet 2 - Splitter-less ADSL, i.e. g.lite */
	G_992_2_POTS_NON_OVERLAPPED		= 9, /* Annex A */
	G_992_2_POTS_OVERLAPPED			= 10, /* Annex B */
	G_992_2_TCM_ISDN_NON_OVERLAPPED	= 11, /* Annex C */
	G_992_2_TCM_ISDN_OVERLAPPED		= 12, /* Annex C */
	/* Bits 13 - 16 are reserved */

	/* Octet 3 - ADSL2 */
	/* Bits 17 - 18 are reserved */
	G_992_3_POTS_NON_OVERLAPPED		= 19, /* Annex A */
	G_992_3_POTS_OVERLAPPED			= 20, /* Annex A */
	G_992_3_ISDN_NON_OVERLAPPED		= 21, /* Annex B */
	G_992_3_ISDN_OVERLAPPED			= 22, /* Annex B */
	G_992_3_TCM_ISDN_NON_OVERLAPPED	= 23, /* Annex C */
	G_992_3_TCM_ISDN_OVERLAPPED		= 24, /* Annex C */

	/* Octet 4 - Splitter-less ADSL2 and ADSL2 */
	G_992_4_POTS_NON_OVERLAPPED		= 25, /* Annex A */
	G_992_4_POTS_OVERLAPPED			= 26, /* Annex A */
	/* Bits 27 - 28 are reserved */
	G_992_3_ANNEX_I_NON_OVERLAPPED	= 29, /* All digital mode */
	G_992_3_ANNEX_I_OVERLAPPED		= 30, /* All digital mode */
	G_992_3_ANNEX_J_NON_OVERLAPPED	= 31, /* All digital mode */
	G_992_3_ANNEX_J_OVERLAPPED		= 32, /* All digital mode */


	/* Octet 5 - Splitter-less ADSL2 and ADSL2 */
	G_992_4_ANNEX_I_NON_OVERLAPPED	= 33, /* All digital mode */
	G_992_4_ANNEX_I_OVERLAPPED		= 34, /* All digital mode */
	G_992_3_POTS_MODE_1				= 35, /* Annex L, non-overlapped, wide upstream */
	G_992_3_POTS_MODE_2				= 36, /* Annex L, non-overlapped, narrow upstream */
	G_992_3_POTS_MODE_3				= 37, /* Annex L, overlapped, wide upstream */
	G_992_3_POTS_MODE_4				= 38, /* Annex L, overlapped, narrow upstream */
	G_992_3_EXT_POTS_NON_OVERLAPPED	= 39, /* Annex M */
	G_992_3_EXT_POTS_OVERLAPPED		= 40, /* Annex M */

	/* Octet 6 - ADSL2+ */
	G_992_5_POTS_NON_OVERLAPPED		= 41, /* Annex A */
	G_992_5_POTS_OVERLAPPED			= 42, /* Annex A */
	G_992_5_ISDN_NON_OVERLAPPED		= 43, /* Annex B */
	G_992_5_ISDN_OVERLAPPED			= 44, /* Annex B */
	G_992_5_TCM_ISDN_NON_OVERLAPPED	= 45, /* Annex C */
	G_992_5_TCM_ISDN_OVERLAPPED		= 46, /* Annex C */
	G_992_5_ANNEX_I_NON_OVERLAPPED	= 47, /* All digital mode */
	G_992_5_ANNEX_I_OVERLAPPED		= 48, /* All digital mode */

	/* Octet 7 - ADSL2+ */
	G_992_5_ANNEX_J_NON_OVERLAPPED	= 49, /* All digital mode */
	G_992_5_ANNEX_J_OVERLAPPED		= 50, /* All digital mode */
	G_992_5_EXT_POTS_NON_OVERLAPPED	= 51, /* Annex M */
	G_992_5_EXT_POTS_OVERLAPPED		= 52, /* Annex M */
	/* Bits 53 - 56 are reserved */

	/* Octet 8 - VDSL2 */
	G_993_2_NORTH_AMERICA			= 57, /* Annex A */
	G_993_2_EUROPE					= 58, /* Annex B */
	G_993_2_JAPAN					= 59, /* Annex C */
	/* Bits 60 - 64 are reserved */
};

/**
 * This macro determines whether a XTSE bit is set
 *
 * @param[in] xtse unsigned char[8] as defined in dsl_line.xtse
 * @param[in] bit Bit number as defined in G.997.1 clause 7.3.1.1.1 XTU transmission system enabling (XTSE)
 *
 * @return A non-zero value is the bit is set. Otherwise 0
 */
#define XTSE_BIT_GET(xtse, bit) (xtse[((bit) - 1) / 8] & (1 << (((bit) - 1) % 8)))
#define XTSE_BIT_SET(xtse, bit) (xtse[((bit) - 1) / 8] |= (1 << (((bit) - 1) % 8)))

/** struct dsl_standard - DSL standards */
struct dsl_standard {
	bool use_xtse; /* true if xtse is used. false if mode is used */
	union {
		/** Bit maps defined in dsl_modtype */
		unsigned long mode;
		/** Transmission system types to be allowed by the xTU on this line instance */
		unsigned char xtse[8];
	};
};

/** enum dsl_line_encoding - Line encoding */
enum dsl_line_encoding {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	LE_DMT = 1,
	LE_CAP,
	LE_2B1Q,
	LE_43BT,
	LE_PAM,
	LE_QAM
};

/** enum dsl_profile - DSL profiles */
enum dsl_profile {
	VDSL2_8a	= 1,
	VDSL2_8b	= 1 << 1,
	VDSL2_8c	= 1 << 2,
	VDSL2_8d	= 1 << 3,
	VDSL2_12a	= 1 << 4,
	VDSL2_12b	= 1 << 5,
	VDSL2_17a	= 1 << 6,
	VDSL2_30a	= 1 << 7,
	VDSL2_35b	= 1 << 8,
};

/** enum dsl_power_state - power states */
enum dsl_power_state {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	DSL_L0 = 1,
	DSL_L1,
	DSL_L2,
	DSL_L3,
	DSL_L4
};

/** struct dsl_line - DSL line parameters */
struct dsl_line {
	/** The current operational state of the DSL line */
	enum itf_status status;
	/** Whether the interface points towards the Internet (true) or towards End Devices (false) */
	bool upstream;
	/** The version of the modem firmware currently installed for this interface */
	char firmware_version[64];
	/** Status of the DSL physical link */
	enum dsl_link_status link_status;
	/** The transmission system types or standards supported */
	struct dsl_standard standard_supported;
	/** The currently used transmission system type, or the standard on this line instance */
	struct dsl_standard standard_used;
	/** The line encoding method used in establishing the Layer 1 DSL connection between the CPE and the DSLAM */
	enum dsl_line_encoding line_encoding;
	/** VDSL2 profiles are allowed on the line. The bitmap is defined in enum dsl_profile */
	unsigned long allowed_profiles;
	/** VDSL2 profile is currently in use on the line */
	enum dsl_profile current_profile;
	/** The power management state of the line */
	enum dsl_power_state power_management_state;
	/** The success failure cause of the initialization */
	unsigned int success_failure_cause;
	/** VTU-R estimated electrical loop length in the range from 0 to 128 dB in steps of 0.1 dB */
	unsigned int upbokler;
	/** VTU-O estimated upstream power back-off electrical length per band */
	dsl_ulong_sequence_t upbokle_pb;
	/** VTU-R estimated upstream power back-off electrical length per band */
	dsl_ulong_sequence_t upbokler_pb;
	/** Downstream receiver signal level threshold */
	dsl_ulong_sequence_t rxthrsh_ds;
	/** The actual active rate adaptation mode in both directions */
	dsl_ulong_t act_ra_mode;
	/** The actual impulse noise protection of the robust overhead channel in both directions */
	dsl_ulong_t act_inp_roc;
	/** The actual signal-to-noise margin of the robust overhead channel (ROC) in both directions */
	dsl_ulong_t snr_mroc;
	/** The last successful transmitted initialization state in both directions */
	dsl_ulong_t last_state_transmitted;
	/** VTU-O estimated electrical loop length in the range from 0 to 128 dB in steps of 0.1 dB */
	unsigned int upbokle;
	/** The enabled VDSL2 Limit PSD mask of the selected PSD mask class */
	unsigned int limit_mask;
	/** The allowed VDSL2 US0 PSD masks for Annex A operation */
	unsigned int us0_mask;
	/** Whether trellis coding is enabled in the downstream and upstream directions */
	dsl_long_t trellis;
	/** Whether the OPTIONAL virtual noise mechanism is in use in both directions */
	dsl_ulong_t act_snr_mode;
	/** The actual cyclic extension */
	unsigned int actual_ce;
	/** The line pair that the modem is using to connection */
	int line_number;
	/** The current maximum attainable data rate in both directions (expressed in Kbps) */
	dsl_ulong_t max_bit_rate;
	/** The current signal-to-noise ratio margin (expressed in 0.1dB) in both directions */
	dsl_long_t noise_margin;
	/** The current signal-to-noise ratio margin of each upstream band */
	dsl_long_sequence_t snr_mpb_us;
	/** The current signal-to-noise ratio margin of each downstream band */
	dsl_long_sequence_t snr_mpb_ds;
	/** The Impulse Noise Monitoring (INM) Inter Arrival Time (IAT) Offset ranges from 3 to 511
	 *  DMT symbols in steps of 1 */
	unsigned int inmiato_ds;
	/** The Impulse Noise Monitoring (INM) Inter Arrival Time (IAT) Step ranges from 0 to 7 in step of 1 */
	unsigned int inmiats_ds;
	/** The Impulse Noise Monitoring (INM) Cluster Continuation (CC) value ranges from 0 to 64
	 *  DMT symbols in steps of 1 */
	unsigned int inmcc_ds;
	/** The Impulse Noise Monitoring (INM) Equivalent Impulse Noise Protection (INP) Mode that
	 *  the xTU receiver uses ranges from 0 to 3 */
	unsigned int inm_inpeq_mode_ds;
	/** The current upstream and downstream signal loss (expressed in 0.1dB). */
	dsl_long_t attenuation;
	/** The current output and received power at the CPE's DSL line (expressed in 0.1dBmV) */
	dsl_long_t power;
	/** xTU-R vendor identifier as defined in G.994.1 and T1.413 in hex binary format */
	char xtur_vendor[8 + 1]; // Adding '\0' in the end for convenient manipulation
	/** T.35 country code of the xTU-R vendor as defined in G.994.1 in hex binary format */
	char xtur_country[4 + 1]; // Adding '\0' in the end for convenient manipulation
	/** xTU-R T1.413 Revision Number as defined in T1.413 Issue 2 */
	unsigned int xtur_ansi_std;
	/** xTU-R Vendor Revision Number as defined in T1.413 Issue 2 */
	unsigned int xtur_ansi_rev;
	/** xTU-C vendor identifier as defined in G.994.1 and T1.413 in hex binary format */
	char xtuc_vendor[8 + 1]; // Adding '\0' in the end for convenient manipulation
	/** T.35 country code of the xTU-C vendor as defined in G.994.1 in hex binary format */
	char xtuc_country[4 + 1]; // Adding '\0' in the end for convenient manipulation
	/** xTU-C T1.413 Revision Number as defined in T1.413 Issue 2 */
	unsigned int xtuc_ansi_std;
	/** xTU-C Vendor Revision Number as defined in T1.413 Issue 2 */
	unsigned int xtuc_ansi_rev;
};

/**
 * struct dsl_line_channel_stats - Statistics counters for DSL line and channel
 * Note that only DSL specific statistics counters are defined since generic statistics are define
 * in an IP interface
 */
struct dsl_line_channel_stats {
	/** The number of seconds since the beginning of the period used for collection of Total statistics */
	unsigned int total_start;
	/** The number of seconds since the most recent DSL Showtime */
	unsigned int showtime_start;
	/** The number of seconds since the second most recent DSL Showtime-the beginning of the period used for
	 *  collection of LastShowtime statistics */
	unsigned int last_showtime_start;
	/** The number of seconds since the beginning of the period used for collection of CurrentDay statistics */
	unsigned int current_day_start;
	/** The number of seconds since the beginning of the period used for collection of QuarterHour statistics */
	unsigned int quarter_hour_start;
};

/** struct dsl_line_stats_interval - This is a common structure for all interval statistics */
struct dsl_line_stats_interval {
	/** Number of errored seconds */
	unsigned int errored_secs;
	/** Number of severely errored seconds */
	unsigned int severely_errored_secs;
};

/**
 *	enum dsl_stats_type - Type of DSL interval statistics
 */
enum dsl_stats_type {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	DSL_STATS_TOTAL = 1,
	DSL_STATS_SHOWTIME,
	DSL_STATS_LASTSHOWTIME,
	DSL_STATS_CURRENTDAY,
	DSL_STATS_QUARTERHOUR
};

/** struct dsl_line_test_params - DSL line test parameters that are available during L0 (Showtime) state */
struct dsl_line_test_params {
	dsl_ulong_t hlogg; /** Number of sub-carriers per sub-carrier group in both directions */
	dsl_ulong_elements_t hlogps_ds; /** Downstream logarithmic line characteristics per sub-carrier group */
	dsl_ulong_elements_t hlogps_us; /** Upstream logarithmic line characteristics per sub-carrier group */
	dsl_ulong_t hlogmt; /** Number of symbols over which HLOGps were measured in both directions */

	dsl_ulong_t qlng; /* Number of sub-carriers per sub-carrier group for QLNps in both directions */
	dsl_ulong_elements_t qlnps_ds; /** Downstream quiet line noise per sub-carrier group */
	dsl_ulong_elements_t qlnps_us; /** Upstream quiet line noise per sub-carrier group */
	dsl_ulong_t qlnmt; /** Number of symbols over which QLNps were measured in both directions */

	dsl_ulong_t snrg; /* Number of sub-carriers per sub-carrier group for SNRps in both directions */
	dsl_ulong_elements_t snrps_ds; /** Downstream SNR per sub-carrier group */
	dsl_ulong_elements_t snrps_us; /** Upstream SNR per sub-carrier group */
	dsl_ulong_t snrmt; /** Number of symbols over which SNRps were measured in both directions */

	/** Downstream line attenuation averaged across all sub-carriers in the frequency band */
	dsl_ulong_elements_t latn_ds;
	/** Upstream line attenuation averaged across all sub-carriers in the frequency band */
	dsl_ulong_elements_t latn_us;

	/** Downstream line attenuation averaged across all active sub-carriers in the frequency band */
	dsl_ulong_elements_t satn_ds;
	/** Upstream line attenuation averaged across all active sub-carriers in the frequency band */
	dsl_ulong_elements_t satn_us;
};

/** struct dsl_line_data_gathering - DSL line data gathering at the VTU-R. This only applies to VDSL2 */
struct dsl_line_data_gathering {
	/** Max depth of entire data gathering event buffer at VTU-R */
	unsigned int logging_depth_r;
	/** Actual depth this is used for reporting data gathering event buffer at VTU-R */
	unsigned int act_logging_depth_r;
};

/** enum dsl_link_encapsulation - Type of link encapsulation method defineds as bit maps */
enum dsl_link_encapsulation {
	G_992_3_ANNEK_K_ATM	= 1,
	G_992_3_ANNEK_K_PTM	= 1 << 1,
	G_993_2_ANNEK_K_ATM	= 1 << 2,
	G_993_2_ANNEK_K_PTM	= 1 << 3,
	G_994_1_AUTO		= 1 << 4
};

/** struct dsl_channel - DSL channel parameters */
struct dsl_channel {
	/** The current operational state of the DSL channel */
	enum itf_status status;
	/** Which link encapsulation standards and recommendations are supported by the channel */
	unsigned long link_encapsulation_supported;
	/** The link encapsulation standard that the channel instance is using for the connection. */
	enum dsl_link_encapsulation link_encapsulation_used;
	/** The index of the latency path supporting the bearer channel */
	unsigned int lpath;
	/** The interleaver depth D for the latency path indicated in lpath */
	unsigned int intlvdepth;
	/** The interleaver block length in use on the latency path indicated in lpath */
	int intlvblock;
	/** The actual delay, in milliseconds, of the latency path due to interleaving */
	unsigned int actual_interleaving_delay;
	/** The actual impulse noise protection (INP) provided by the latency path indicated in lpath */
	int actinp;
	/** Whether the value reported in actinp was computed assuming the receiver does not use erasure decoding */
	bool inpreport;
	/** Reports the size, in octets, of the Reed-Solomon codeword in use on the latency path indicated in lpath */
	int nfec;
	/** The number of redundancy bytes per Reed-Solomon codeword on the latency path indicated in lpath */
	int rfec;
	/** The number of bits per symbol assigned to the latency path indicated in lpath */
	int lsymb;
	/** The current physical layer aggregate data rate (expressed in Kbps) in both directions. */
	dsl_ulong_t curr_rate;
	/** Actual net data rate expressed in Kbps in both directions */
	dsl_ulong_t actndr;
	/** Actual impulse noise protection in both directions against REIN, expressed in 0.1 DMT symbols */
	dsl_ulong_t actinprein;
};

/** This value shall be used if any of elements defined in "struct dsl_channel_stats_interval" is unavailable */
#define DSL_INVALID_STATS_COUNTER 4294967295
/**
 *  struct dsl_channel_stats_interval - This is a common structure for all interval statistics
 */
struct dsl_channel_stats_interval {
	/** Number of FEC errors */
	unsigned int xtur_fec_errors;
	/** Number of FEC errors detected by the ATU-C */
	unsigned int xtuc_fec_errors;
	/** Number of HEC errors */
	unsigned int xtur_hec_errors;
	/** Number of HEC errors detected by the ATU-C */
	unsigned int xtuc_hec_errors;
	/** Number of CRC errors */
	unsigned int xtur_crc_errors;
	/** Number of CRC errors detected by the ATU-C */
	unsigned int xtuc_crc_errors;
};

/** struct adsl_line_test - Diagnostic information for ADSL, ADSL2 and ADSL2+ */
struct adsl_line_test {
	dsl_long_t act_psd; /** Actual power spectral density in both directions */
	dsl_long_t act_atp; /** Actual aggregate transmission power in both directions */

	dsl_long_t hlinsc; /** Linear representation scales in both directions */
	dsl_ulong_t hling; /** Number of sub-carriers per sub-carrier group for HLINps in both directions */

	dsl_ulong_t hlogg; /** Number of sub-carriers per sub-carrier group for HLOGps in both directions */
	dsl_ulong_elements_t hlogps_ds; /** Downstream logarithmic channel characteristics per sub-carrier group */
	dsl_ulong_elements_t hlogps_us; /** Upstream logarithmic channel characteristics per sub-carrier group */
	dsl_ulong_t hlogmt; /** Number of symbols over which HLOGps were measured in both directions */

	dsl_ulong_sequence_t latnpb_ds; /** Downstream line attenuation per usable band during initialization */
	dsl_ulong_sequence_t latnpb_us; /** Upstream line attenuation per usable band during initialization */

	dsl_ulong_sequence_t satn_ds; /** Downstream line attenuation per usable band during showtime state */
	dsl_ulong_sequence_t satn_us; /** Upstream line attenuation per usable band during showtime state */

	dsl_ulong_elements_t hlinps_ds; /** Downstream linear channel characteristics per sub-carrier group */
	dsl_ulong_elements_t hlinps_us; /** Upstream linear channel characteristics per sub-carrier group */

	dsl_ulong_t qlng; /** Number of sub-carriers per sub-carrier group in both directions */
	dsl_ulong_elements_t qlnps_ds; /** Downstream quiet line noise per sub-carrier group */
	dsl_ulong_elements_t qlnps_us; /** Upstream quiet line noise per sub-carrier group */

	dsl_ulong_t qlnmt; /** Number of symbols over which QLNps were measured in both directions */

	dsl_ulong_t snrg; /** Number of sub-carriers per sub-carrier group for SNRps in both directions */
	dsl_ulong_elements_t snrps_ds; /** Downstream SNR per sub-carrier group */
	dsl_ulong_elements_t snrps_us; /** Upstream SNR per sub-carrier group */

	dsl_ulong_t snrmt; /** Number of symbols over which SNRps were measured in both directions */

	dsl_ulong_elements_t bitsps_ds; /** Downstream bit allocation per sub-carrier group */
	dsl_ulong_elements_t bitsps_us; /** Upstream bit allocation per sub-carrier group */
};

/**
 *  struct dsl_seltuer - DSL Single Ended Line Test - Physical Medium Dependent (SELT-PMD) Uncalibrated Echo
 *  Response (UER) diagnostic parameters
 */
struct dsl_seltuer {
	unsigned int uer_max_duration; /** RW SELT UER maximum measurement duration (MMD) in seconds */
	bool ext_bandwidth_op; /** RO Whether the option of extended bandwidth SELT is applied */
	dsl_ulong_elements_t uer; /** RO List of UER Complex */
	unsigned int uer_scale_factor;  /** RO UER scale factor */
	unsigned int uer_group_size; /** RO UER group size in units of sub-carriers */
	dsl_ulong_elements_t uer_var; /** RO Variance of UER */
};

/**
 *  struct dsl_seltqln - DSL Single Ended Line Test - Physical Medium Dependent (SELT-PMD) Quiet Line
 *  Noise (QLN) diagnostic parameters
 */
struct dsl_seltqln {
	unsigned int qln_max_measurement_duration;  /** RW SELT QLN max measurement duration in seconds */
	bool ext_bandwidth_op; /** RO Whether the option of extended bandwidth SELT is applied */
	dsl_ulong_elements_t qln; /** RO List represents  SELT quiet line noise per sub-carrier group */
	unsigned int qln_group_size; /** RO QLN group size in units of sub-carriers */
};

/** enum dsl_loop_termination - DSL loop termination indicator */
enum dsl_loop_termination {
	OPEN,
	SHORT,
	POWERED_ON,
	UNKNOWN
};

/**
 *  struct dsl_seltp - DSL Single Ended Line Test - Processed diagnostic parameters
 */
struct dsl_seltp {
	bool capacity_estimate_enable; /** RW Whether DSL performance estimation is required */
	dsl_ulong_elements_t capacity_signal_psd; /** RW Capacity estimate signal Power Spectral Density */
	dsl_ulong_elements_t capacity_noise_psd; /** RW Capacity estimate noise Power Spectral Density */
	unsigned int capacity_target_margin; /** RW Capacity estimate target noise margin */
	enum dsl_loop_termination loop_term; /** RO Loop termination indicator */
	unsigned int loop_length; /** RO Loop length in units of meters */
	struct {
		struct {
			unsigned int loop_seg_len;
			unsigned int loop_seg_btap;
		} pairs[256];
		int count; /* Number of items in pairs */
	} loop_topology; /** RO Loop topology */
	dsl_ulong_elements_t attenuation_characteristics; /** RO TFlog(f) parameter defined in G.996.2 */
	bool missing_filter; /** RO Missing micro-filter or splitter */
	unsigned int capacity_estimate; /** RO Capacity estimate in kbit/s */
};

/** enum fast_profile - FAST profile
 *  TR-181 only lists 106a and 212a as possible profiles, but Broadcom supports these
 */
enum fast_profile {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	FAST_106a = 1,
	FAST_106b = 1 << 1,
	FAST_106c = 1 << 2,
	FAST_212a = 1 << 3,
	FAST_212c = 1 << 4
};

/** enum fast_power_state - power states */
enum fast_power_state {
	/* Starts with non-zero in order to distinguish from an uninitialized value which is usually 0 */
	FAST_L0 = 1,
	FAST_L2_1,
	FAST_L2_2,
	FAST_L3
};

/** struct fast_line - Physical FAST line */
struct fast_line {
	/** The current operational state of the FAST line */
	enum itf_status status;
	/** Whether the interface points towards the Internet (true) or towards End Devices (false) */
	bool upstream;
	/** The version of the modem firmware currently installed for this interface */
	char firmware_version[64];
	/** Status of the FAST physical link */
	enum dsl_link_status link_status;
	/** FAST profiles are allowed on the line. The bitmap is defined in enum fast_profile */
	unsigned long allowed_profiles;
	/** FAST profile is currently in use on the line */
	enum fast_profile current_profile;
	/** The power management state of the line */
	enum fast_power_state power_management_state;
	/** The success failure cause of the initialization */
	unsigned int success_failure_cause;
	/** FTU-R estimated electrical length in the range from 0 to 128 dB in steps of 0.1 dB */
	unsigned int upbokler;
	/** The signal count of the last transmitted initialization signal in both directions */
	dsl_ulong_t last_transmitted_signal;
	/** FTU-O estimated electrical length in the range from 0 to 128 dB in steps of 0.1 dB */
	unsigned int upbokle;
	/** The line pair that the modem is using to connection */
	int line_number;
	/** The attainable net data rate in both directions (expressed in Kbps) */
	dsl_ulong_t max_bit_rate;
	/** The current signal-to-noise ratio margin (expressed in 0.1dB) in both directions */
	dsl_long_t noise_margin;
	/** The current upstream and downstream signal loss (expressed in 0.1dB) */
	dsl_long_t attenuation;
	/** The current output and received power at the CPE's FAST line (expressed in 0.1dBmV) */
	dsl_long_t power;
	/** The signal-to-noise ratio margin (SNRM, expressed in 0.1dB) for the robust management
	 *  channel (RMC) in both directions */
	dsl_long_t snrm_rmc;
	/** Bit allocation values on RMC sub-carriers in RMC symbols in both directions */
	dsl_ulong_elements_t bits_rmc_ps_ds;
	dsl_ulong_elements_t bits_rmc_ps_us;
	/** Whether FEXT cancellation from all the other vectored lines into the line in the vectored group
	 *  is enabled in both directions */
	dsl_bool_t fext_to_cancel_enable;
	/** Expected Throughput Rate in Kbps in both directions */
	dsl_ulong_t etr;
	/** Attainable Expected Throughput Rate in Kbps in both directions */
	dsl_ulong_t att_etr;
	/** Minimum Error Free Expected Throughput Rate in Kbps */
	dsl_ulong_t min_eftr;
};

/** Keep the backward compatibility in case statistics counters for xDSL and FAST diverge in the future */
#define fast_line_stats dsl_line_channel_stats

/** struct fast_line_stats_interval - This is a common structure for all interval statistics for FAST line */
struct fast_line_stats_interval {
	/** Number of errored seconds */
	unsigned int errored_secs;
	/** Number of severely errored seconds */
	unsigned int severely_errored_secs;
	/** Total number of loss of signal seconds */
	unsigned int loss;
	/** Total number of loss of RMC seconds */
	unsigned int lors;
	/** Total number unavailable seconds */
	unsigned int uas;
	/** Count of uncorrected DTU anomalies */
	unsigned int rtx_uc;
	/** Count of retransmitted DTU anomalies */
	unsigned int rtx_tx;
	/** Count of successful bit swap (BSW) primitives */
	unsigned int success_bsw;
	/** Count of successful autonomous SRA (Seamless Rate Adaptation) primitives */
	unsigned int success_sra;
	/** Count of successful FRA (Fast Rate Adaptation) primitives */
	unsigned int success_fra;
	/** Count of successful RPA (RMC Parameter Adjustment) primitives */
	unsigned int success_rpa;
	/** Count of successful (Transmitter Initiated Gain Adjustment) primitives */
	unsigned int success_tiga;
};

/** struct fast_line_test_params - FAST line test parameters available during L0 state */
struct fast_line_test_params {
	/* Number of sub-carriers in any one sub-carrier group used to represent the SNR(f)
	 * values in both directions */
	dsl_ulong_t snrg;
	dsl_ulong_elements_t snrps_ds; /** Downstream SNR(f) per sub-carrier group */
	dsl_ulong_elements_t snrps_us; /** Upstream SNR(f) per sub-carrier group */
	dsl_ulong_t snrmt; /** Number of symbols used to measure SNR(f) in both directions */

	unsigned int act_inp; /** Actual INP against SHINE */
	unsigned int nfec; /** DTU FEC codeword length (expressed in 1 byte unit) */
	int rfec; /** DTU FEC codeword redundancy */
	dsl_ulong_t curr_rate; /** Current physical layer aggregate data rate in Kbps in both directions */
	unsigned int act_inp_rein; /** Actual INP against REIN */
};

/** struct dsl_config_params - Configuration parameters of xDSL and FAST */
struct dsl_config_params {
	/** xDSL transmission system types to be allowed by the xTU */
	unsigned char xtse[8];
	/** VDSL2 profiles are allowed on the line. The bitmap is defined in enum dsl_profile */
	unsigned long vdsl2_profiles;
	/** FAST profiles are allowed on the line. The bitmap is defined in enum fast_profile */
	unsigned long fast_profiles;
	/** Enable or disable data gathering on the xDSL line */
	bool enable_data_gathering;
	/** The enabled VDSL2 Limit PSD mask of the selected PSD mask class */
	unsigned int limit_mask;
	/** The allowed VDSL2 US0 PSD masks for Annex A operation */
	unsigned int us0_mask;
};

/**
 * This function gets the number of DSL lines
 *
 * @return the number of DSL lines on success. Otherwise a negative value is returned.
 *
 * Note that this API must be implemented on all platforms on which libdsl is enabled.
 */
int dsl_get_line_number(void);

/**
 * This function gets the number of DSL channels
 *
 * @return the number of DSL channels on success. Otherwise a negative value is returned.
 *
 * Note that this API must be implemented on all platforms on which libdsl is enabled.
 */
int dsl_get_channel_number(void);

/**
 * This function gets the DSL line information
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] line - The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_line_info(int line_num, struct dsl_line *line);

/**
 * This function gets the statistics counters of a DSL line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_line_stats(int line_num, struct dsl_line_channel_stats *stats);

/**
 * This function gets the interval statistics counters of a DSL line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[in] type The type of interval statistics
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct dsl_line_stats_interval *stats);

/**
 * This function gets the test parameters of a DSL line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] test_params The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_line_test_params(int line_num, struct dsl_line_test_params *test_params);

/**
 * This function gets the data gathering parameters of a DSL line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] data_gathering The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_line_data_gathering(int line_num, struct dsl_line_data_gathering *data_gathering);

/**
 * This function gets the DSL channel information
 *
 * @param[in] chan_num - The channel number which starts with 0
 * @param[out] channel The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_channel_info(int chan_num, struct dsl_channel *channel);

/**
 * This function gets the statistics counters of a DSL channel
 *
 * @param[in] chan_num - The channel number which starts with 0
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_channel_stats(int chan_num, struct dsl_line_channel_stats *stats);

/**
 * This function gets the interval statistics counters of a DSL channel
 *
 * @param[in] chan_num - The channel number which starts with 0
 * @param[in] type The type of interval statistics
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_get_channel_stats_interval(int chan_num, enum dsl_stats_type type, struct dsl_channel_stats_interval *stats);

/**
 * This function starts or cancels an ADSL line test
 *
 * @param[in] line_num - The ADSL line number which starts with 0
 * @param[in] start - True to start a test. False to cancel an ongoing a test if any.
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int adsl_line_test(int line_num, bool start);

/**
 * This function gets the results of an ADSL line test
 *
 * @param[in] line_num - The ADSL line number which starts with 0
 * @param[out] results - Output parameter to receive ADSL line test results
 *
 * @return The diagnostic state of an ADSL line test
 */
enum diag_state adsl_get_line_test_results(int line_num, struct adsl_line_test *results);

/**
 * This function starts or cancels a diagnostic SELTUER on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[in] seltuer - Diagnostic SELTUER settings. Note that those RO (Read Only) members in
 *                       struct dsl_seltuer will be ignored
 * @param[in] start - True to start a test. False to cancel an ongoing a test if any.
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_seltuer_test(int line_num, const struct dsl_seltuer *seltuer, bool start);

/**
 * This function gets the results of a diagnostic SELTUER on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[out] seltuer - Output parameter to receive the test results
 *
 * @return The diagnostic state of the test
 */
enum diag_state dsl_get_seltuer_results(int line_num, struct dsl_seltuer *seltuer);

/**
 * This function starts or cancels a diagnostic SELTQLN on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[in] seltqln - Diagnostic SELTQLN settings. Note that those RO (Read Only) members in
 *                       struct dsl_seltqln will be ignored
 * @param[in] start - True to start a test. False to cancel an ongoing a test if any.
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_seltqln_test(int line_num, const struct dsl_seltqln *seltqln, bool start);

/**
 * This function gets the results of a diagnostic SELTQLN on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[out] seltqln - Output parameter to receive the test results
 *
 * @return The diagnostic state of the test
 */
enum diag_state dsl_get_seltqln_results(int line_num, struct dsl_seltqln *seltqln);

/**
 * This function starts or cancels a diagnostic SELTP on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[in] seltp - Diagnostic SELTP settings. Note that those RO (Read Only) members in
 *                       struct dsl_seltp will be ignored
 * @param[in] start - True to start a test. False to cancel an ongoing a test if any.
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_seltp_test(int line_num, const struct dsl_seltp *seltp, bool start);

/**
 * This function gets the results of a diagnostic SELTP on a DSL line
 *
 * @param[in] line_num - The DSL line number which starts with 0
 * @param[out] seltp - Output parameter to receive the test results
 *
 * @return The diagnostic state of the test
 */
enum diag_state dsl_get_seltp_results(int line_num, struct dsl_seltp *seltp);

/**
 * This function gets the FAST line information
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] line - The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int fast_get_line_info(int line_num, struct fast_line *line);

/**
 * This function gets the statistics counters of a FAST line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int fast_get_line_stats(int line_num, struct fast_line_stats *stats);

/**
 * This function gets the interval statistics counters of a FAST line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[in] type The type of interval statistics
 * @param[out] stats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int fast_get_line_stats_interval(int line_num, enum dsl_stats_type type, struct fast_line_stats_interval *stats);

/**
 * This function gets the test parameters of a FAST line
 *
 * @param[in] line_num - The line number which starts with 0
 * @param[out] test_params The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int fast_get_line_test_params(int line_num, struct fast_line_test_params *test_params);


/**
 * This function configures an xDSL or FAST line
 *
 * @param[in] line_num   - The line number which starts with 0
 * @param[in] cfg_params - The configuration parameters to be applied. Depending upon the capabilities of
 *                         the platform, not all parameters in cfg_params are applicable
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_configure(int line_num, struct dsl_config_params *cfg_params);

/**
 * This function sets xDSL OEM parameters
 *
 * @param[in] param_id   - The OEM parameter to set (see bcmadsl.h for reference)
 * @param[in] buf        - The string value to set the parameter to
 * @param[in] len        - The length of the value string
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_set_oem_parameter(int param_id, void *buf, int len);

/**
 *  struct dsl_ops - This structure defines the DSL and FAST operations.
 *  A function pointer shall be NULL if the corresponding operation is not supported on a specific platform
 */
struct dsl_ops {
	/** Status and statistics of an xDSL line or channel */
	int (*get_line_info)(int line_num, struct dsl_line *line);
	int (*get_line_stats)(int line_num, struct dsl_line_channel_stats *stats);
	int (*get_line_stats_interval)(int line_num, enum dsl_stats_type type,
			struct dsl_line_stats_interval *stats);
	int (*get_channel_info)(int chan_num, struct dsl_channel *channel);
	int (*get_channel_stats)(int chan_num, struct dsl_line_channel_stats *stats);
	int (*get_channel_stats_interval)(int chan_num, enum dsl_stats_type type,
			struct dsl_channel_stats_interval *stats);

	/** Diagnostics and test of an xDSL line or channel */
	int (*get_line_test_params)(int line_num, struct dsl_line_test_params *test_params);
	int (*get_line_data_gathering)(int line_num, struct dsl_line_data_gathering *data_gathering);
	int (*line_test)(int line_num, bool start);
	enum diag_state (*get_line_test_results)(int line_num, struct adsl_line_test *results);
	int (*seltuer_test)(int line_num, const struct dsl_seltuer *seltuer, bool start);
	enum diag_state (*get_seltuer_results)(int line_num, struct dsl_seltuer *seltuer);
	int (*seltqln_test)(int line_num, const struct dsl_seltqln *seltqln, bool start);
	enum diag_state (*get_seltqln_results)(int line_num, struct dsl_seltqln *seltqln);
	int (*seltp_test)(int line_num, const struct dsl_seltp *seltp, bool start);
	enum diag_state (*get_seltp_results)(int line_num, struct dsl_seltp *seltp);

	/** Status and statistics of a FAST line */
	int (*get_fast_line_info)(int line_num, struct fast_line *line);
	int (*get_fast_line_stats)(int line_num, struct fast_line_stats *stats);
	int (*get_fast_line_stats_interval)(int line_num, enum dsl_stats_type type,
			struct fast_line_stats_interval *stats);

	/** Diagnostics and test of a FAST line */
	int (*get_fast_line_test_params)(int line_num, struct fast_line_test_params *test_params);

	/** DSL and/or FAST configuration */
	int (*configure)(int line_num, struct dsl_config_params *cfg_params);

	/** Set DSL OEM parameters */
	int (*set_oem_parameter)(int param_id, void *buf, int len);
};

/** This global variable must be defined for each platform specific implementation */
extern const struct dsl_ops xdsl_ops;

/* ====================================== Helper utilities ============================================== */

/**
 * This function allocates the memory for the structure dsl_ulong_elements_t. dsl_free_ulong_elements() shall
 * be used to free the memory after use.
 *
 * @param[out] element  The structure that needs memory allocation
 * @param[in]  count    The number of elements
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_alloc_ulong_elements(dsl_ulong_elements_t *element, int count);

/**
 * This function frees the memory of a structure dsl_ulong_elements_t resets its members properly.
 *
 * @param[out] element  The structure to be free memory and reset
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int dsl_free_ulong_elements(dsl_ulong_elements_t *element);


#ifdef __cplusplus
}
#endif
#endif /* _XDSL_H */
