/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef _RTW_HT_H_
#define _RTW_HT_H_


struct ht_priv {
	u32	ht_option;
	u32	ampdu_enable;//for enable Tx A-MPDU
	//u8	baddbareq_issued[16];
	u32	tx_amsdu_enable;//for enable Tx A-MSDU
	u32	tx_amsdu_maxlen; // 1: 8k, 0:4k ; default:8k, for tx
	u32	rx_ampdu_maxlen; //for rx reordering ctrl win_sz, updated when join_callback.

	u8	bwmode;//
	u8	ch_offset;//PRIME_CHNL_OFFSET
	u8	sgi;//short GI

	//for processing Tx A-MPDU
	u8	agg_enable_bitmap;
	//u8	ADDBA_retry_count;
	u8	candidate_tid_bitmap;

	u8	ldpc_cap;
	u8	stbc_cap;
	u8	beamform_cap;

	struct rtw_ieee80211_ht_cap ht_cap;

};

typedef enum AGGRE_SIZE {
	HT_AGG_SIZE_8K = 0,
	HT_AGG_SIZE_16K = 1,
	HT_AGG_SIZE_32K = 2,
	HT_AGG_SIZE_64K = 3,
	VHT_AGG_SIZE_128K = 4,
	VHT_AGG_SIZE_256K = 5,
	VHT_AGG_SIZE_512K = 6,
	VHT_AGG_SIZE_1024K = 7,
} AGGRE_SIZE_E, *PAGGRE_SIZE_E;

typedef enum _RT_HT_INF0_CAP {
	RT_HT_CAP_USE_TURBO_AGGR = 0x01,
	RT_HT_CAP_USE_LONG_PREAMBLE = 0x02,
	RT_HT_CAP_USE_AMPDU = 0x04,
	RT_HT_CAP_USE_WOW = 0x8,
	RT_HT_CAP_USE_SOFTAP = 0x10,
	RT_HT_CAP_USE_92SE = 0x20,
	RT_HT_CAP_USE_88C_92C = 0x40,
	RT_HT_CAP_USE_AP_CLIENT_MODE = 0x80,	// AP team request to reserve this bit, by Emily
} RT_HT_INF0_CAPBILITY, *PRT_HT_INF0_CAPBILITY;

typedef enum _RT_HT_INF1_CAP {
	RT_HT_CAP_USE_VIDEO_CLIENT = 0x01,
	RT_HT_CAP_USE_JAGUAR_BCUT = 0x02,
	RT_HT_CAP_USE_JAGUAR_CCUT = 0x04,
} RT_HT_INF1_CAPBILITY, *PRT_HT_INF1_CAPBILITY;

#define	LDPC_HT_ENABLE_RX			BIT0
#define	LDPC_HT_ENABLE_TX			BIT1
#define	LDPC_HT_TEST_TX_ENABLE		BIT2
#define	LDPC_HT_CAP_TX				BIT3

#define	STBC_HT_ENABLE_RX			BIT0
#define	STBC_HT_ENABLE_TX			BIT1
#define	STBC_HT_TEST_TX_ENABLE		BIT2
#define	STBC_HT_CAP_TX				BIT3

#define	BEAMFORMING_HT_BEAMFORMER_ENABLE	BIT0	// Declare our NIC supports beamformer
#define	BEAMFORMING_HT_BEAMFORMEE_ENABLE	BIT1	// Declare our NIC supports beamformee
#define	BEAMFORMING_HT_BEAMFORMER_TEST		BIT2	// Transmiting Beamforming no matter the target supports it or not

#endif	//_RTL871X_HT_H_

