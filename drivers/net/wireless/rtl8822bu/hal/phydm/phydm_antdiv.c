/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

/* ******************************************************
 * when antenna test utility is on or some testing need to disable antenna diversity
 * call this function to disable all ODM related mechanisms which will switch antenna.
 * ****************************************************** */
void odm_stop_antenna_switch_dm(
	void			*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	/* disable ODM antenna diversity */
	dm->support_ability &= ~ODM_BB_ANT_DIV;
	odm_ant_div_on_off(dm, ANTDIV_OFF);
	odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
	PHYDM_DBG(dm, DBG_ANT_DIV, "STOP Antenna Diversity\n");
}

void phydm_enable_antenna_diversity(
	void			*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	dm->support_ability |= ODM_BB_ANT_DIV;
	dm->antdiv_select = 0;
	PHYDM_DBG(dm, DBG_ANT_DIV, "AntDiv is enabled & Re-Init AntDiv\n");
	odm_antenna_diversity_init(dm);
}

void odm_set_ant_config(
	void	*dm_void,
	u8		ant_setting	/* 0=A, 1=B, 2=C, .... */
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	if (dm->support_ic_type == ODM_RTL8723B) {
		if (ant_setting == 0)		/* ant A*/
			odm_set_bb_reg(dm, 0x948, MASKDWORD, 0x00000000);
		else if (ant_setting == 1)
			odm_set_bb_reg(dm, 0x948, MASKDWORD, 0x00000280);
	} else if (dm->support_ic_type == ODM_RTL8723D) {
		if (ant_setting == 0)		/* ant A*/
			odm_set_bb_reg(dm, 0x948, MASKLWORD, 0x0000);
		else if (ant_setting == 1)
			odm_set_bb_reg(dm, 0x948, MASKLWORD, 0x0280);
	}
}

/* ****************************************************** */


void odm_sw_ant_div_rest_after_link(
	void		*dm_void
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;
	u32	i;

	if (dm->ant_div_type == S0S1_SW_ANTDIV) {
		dm_swat_table->try_flag = SWAW_STEP_INIT;
		dm_swat_table->rssi_trying = 0;
		dm_swat_table->double_chk_flag = 0;
		fat_tab->rx_idle_ant = MAIN_ANT;

		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++)
			phydm_antdiv_reset_statistic(dm, i);
	}

#endif
}

void odm_ant_div_on_off(
	void		*dm_void,
	u8		swch
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	if (fat_tab->ant_div_on_off != swch) {
		if (dm->ant_div_type == S0S1_SW_ANTDIV)
			return;

		if (dm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "(( Turn %s )) N-Series HW-AntDiv block\n",
				  (swch == ANTDIV_ON) ? "ON" : "OFF");
			odm_set_bb_reg(dm, 0xc50, BIT(7), swch);
			odm_set_bb_reg(dm, 0xa00, BIT(15), swch);

#if (RTL8723D_SUPPORT == 1)
			/*Mingzhi 2017-05-08*/
			if (dm->support_ic_type == ODM_RTL8723D) {
				if (swch == ANTDIV_ON) {
					odm_set_bb_reg(dm, 0xce0, BIT(1), 1);
					odm_set_bb_reg(dm, 0x948, BIT(6), 1);          /*1:HW ctrl  0:SW ctrl*/
				} else {
					odm_set_bb_reg(dm, 0xce0, BIT(1), 0);
					odm_set_bb_reg(dm, 0x948, BIT(6), 0);          /*1:HW ctrl  0:SW ctrl*/
				}
			}
#endif

		} else if (dm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "(( Turn %s )) AC-Series HW-AntDiv block\n",
				  (swch == ANTDIV_ON) ? "ON" : "OFF");
			if (dm->support_ic_type & ODM_RTL8812) {
				odm_set_bb_reg(dm, 0xc50, BIT(7),
					       swch); /* OFDM AntDiv function block enable */
				odm_set_bb_reg(dm, 0xa00, BIT(15),
					       swch); /* CCK AntDiv function block enable */
			} else {
				odm_set_bb_reg(dm, 0x8D4, BIT(24),
					       swch); /* OFDM AntDiv function block enable */

				if ((dm->cut_version >= ODM_CUT_C) && (dm->support_ic_type == ODM_RTL8821)
				    && (dm->ant_div_type != S0S1_SW_ANTDIV)) {
					PHYDM_DBG(dm, DBG_ANT_DIV, "(( Turn %s )) CCK HW-AntDiv block\n",
						  (swch == ANTDIV_ON) ? "ON" : "OFF");
					odm_set_bb_reg(dm, 0x800, BIT(25), swch);
					odm_set_bb_reg(dm, 0xA00, BIT(15),
						       swch); /* CCK AntDiv function block enable */
				} else if (dm->support_ic_type == ODM_RTL8821C) {
					PHYDM_DBG(dm, DBG_ANT_DIV, "(( Turn %s )) CCK HW-AntDiv block\n",
						  (swch == ANTDIV_ON) ? "ON" : "OFF");
					odm_set_bb_reg(dm, 0x800, BIT(25), swch);
					odm_set_bb_reg(dm, 0xA00, BIT(15),
						       swch); /* CCK AntDiv function block enable */
				}
			}
		}
	}
	fat_tab->ant_div_on_off = swch;

}

void odm_tx_by_tx_desc_or_reg(
	void		*dm_void,
	u8			swch
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;
	u8 enable;

	if (fat_tab->b_fix_tx_ant == NO_FIX_TX_ANT)
		enable = (swch == TX_BY_DESC) ? 1 : 0;
	else
		enable = 0;/*Force TX by Reg*/

	if (dm->ant_div_type != CGCS_RX_HW_ANTDIV) {
		if (dm->support_ic_type & ODM_N_ANTDIV_SUPPORT)
			odm_set_bb_reg(dm, 0x80c, BIT(21), enable);
		else if (dm->support_ic_type & ODM_AC_ANTDIV_SUPPORT)
			odm_set_bb_reg(dm, 0x900, BIT(18), enable);

		PHYDM_DBG(dm, DBG_ANT_DIV, "[AntDiv] TX_Ant_BY (( %s ))\n",
			  (enable == TX_BY_DESC) ? "DESC" : "REG");
	}
}

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
void phydm_antdiv_reset_statistic(
	void	*dm_void,
	u32	macid
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	fat_tab->main_ant_sum[macid] = 0;
	fat_tab->aux_ant_sum[macid] = 0;
	fat_tab->main_ant_cnt[macid] = 0;
	fat_tab->aux_ant_cnt[macid] = 0;
	fat_tab->main_ant_sum_cck[macid] = 0;
	fat_tab->aux_ant_sum_cck[macid] = 0;
	fat_tab->main_ant_cnt_cck[macid] = 0;
	fat_tab->aux_ant_cnt_cck[macid] = 0;
}

void phydm_fast_training_enable(
	void		*dm_void,
	u8			swch
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u8			enable;

	if (swch == FAT_ON)
		enable = 1;
	else
		enable = 0;

	PHYDM_DBG(dm, DBG_ANT_DIV, "Fast ant Training_en = ((%d))\n", enable);

	if (dm->support_ic_type == ODM_RTL8188E) {
		odm_set_bb_reg(dm, 0xe08, BIT(16), enable);	/*enable fast training*/
		/**/
	} else if (dm->support_ic_type == ODM_RTL8192E) {
		odm_set_bb_reg(dm, 0xB34, BIT(28),
			       enable);	/*enable fast training (path-A)*/
		/*odm_set_bb_reg(dm, 0xB34, BIT(29), enable);*/	/*enable fast training (path-B)*/
	} else if (dm->support_ic_type & (ODM_RTL8821 | ODM_RTL8822B)) {
		odm_set_bb_reg(dm, 0x900, BIT(19), enable);	/*enable fast training */
		/**/
	}
}

void phydm_keep_rx_ack_ant_by_tx_ant_time(
	void		*dm_void,
	u32		time
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	/* Timming issue: keep Rx ant after tx for ACK ( time x 3.2 mu sec)*/
	if (dm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {
		odm_set_bb_reg(dm, 0xE20, BIT(23) | BIT(22) | BIT(21) | BIT(20), time);
		/**/
	} else if (dm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {
		odm_set_bb_reg(dm, 0x818, BIT(23) | BIT(22) | BIT(21) | BIT(20), time);
		/**/
	}
}

void odm_update_rx_idle_ant(
	void		*dm_void,
	u8		ant
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	u32			default_ant, optional_ant, value32, default_tx_ant;

	if (fat_tab->rx_idle_ant != ant) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ Update Rx-Idle-ant ] rx_idle_ant =%s\n",
			  (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

		if (!(dm->support_ic_type & ODM_RTL8723B))
			fat_tab->rx_idle_ant = ant;

		if (ant == MAIN_ANT) {
			default_ant   =  ANT1_2G;
			optional_ant =  ANT2_2G;
		} else {
			default_ant  =   ANT2_2G;
			optional_ant =  ANT1_2G;
		}

		if (fat_tab->b_fix_tx_ant != NO_FIX_TX_ANT)
			default_tx_ant = (fat_tab->b_fix_tx_ant == FIX_TX_AT_MAIN) ? 0 : 1;
		else
			default_tx_ant = default_ant;

		if (dm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {
			if (dm->support_ic_type == ODM_RTL8192E) {
				odm_set_bb_reg(dm, 0xB38, BIT(5) | BIT(4) | BIT(3),
					       default_ant); /* Default RX */
				odm_set_bb_reg(dm, 0xB38, BIT(8) | BIT(7) | BIT(6),
					       optional_ant); /* Optional RX */
				odm_set_bb_reg(dm, 0x860, BIT(14) | BIT(13) | BIT(12),
					       default_ant); /* Default TX */
			}
#if (RTL8723B_SUPPORT == 1)
			else if (dm->support_ic_type == ODM_RTL8723B) {
				value32 = odm_get_bb_reg(dm, 0x948, 0xFFF);

				if (value32 != 0x280)
					odm_update_rx_idle_ant_8723b(dm, ant, default_ant, optional_ant);
				else
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ Update Rx-Idle-ant ] 8723B: Fail to set RX antenna due to 0x948 = 0x280\n");
			}
#endif

#if (RTL8723D_SUPPORT == 1)         /*Mingzhi 2017-05-08*/
			else if (dm->support_ic_type == ODM_RTL8723D) {
				phydm_set_tx_ant_pwr_8723d(dm, ant);
				odm_update_rx_idle_ant_8723d(dm, ant, default_ant, optional_ant);

			}
#endif

			else { /*8188E & 8188F*/
				/*
								if (dm->support_ic_type == ODM_RTL8723D) {
				#if (RTL8723D_SUPPORT == 1)
									phydm_set_tx_ant_pwr_8723d(dm, ant);
				#endif
								}
				*/
#if (RTL8188F_SUPPORT == 1)
				if (dm->support_ic_type == ODM_RTL8188F) {
					phydm_update_rx_idle_antenna_8188F(dm, default_ant);
					/**/
				}
#endif

				odm_set_bb_reg(dm, 0x864, BIT(5) | BIT(4) | BIT(3),
					       default_ant);		/*Default RX*/
				odm_set_bb_reg(dm, 0x864, BIT(8) | BIT(7) | BIT(6),
					       optional_ant);	/*Optional RX*/
				odm_set_bb_reg(dm, 0x860, BIT(14) | BIT(13) | BIT(12),
					       default_tx_ant);	/*Default TX*/
			}
		} else if (dm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {
			u16	value16 = odm_read_2byte(dm, ODM_REG_TRMUX_11AC + 2);
			/*  */
			/* 2014/01/14 MH/Luke.Lee Add direct write for register 0xc0a to prevnt */
			/* incorrect 0xc08 bit0-15 .We still not know why it is changed. */
			/*  */
			value16 &= ~(BIT(11) | BIT(10) | BIT(9) | BIT(8) | BIT(7) | BIT(6) | BIT(
					     5) | BIT(4) | BIT(3));
			value16 |= ((u16)default_ant << 3);
			value16 |= ((u16)optional_ant << 6);
			value16 |= ((u16)default_ant << 9);
			odm_write_2byte(dm, ODM_REG_TRMUX_11AC + 2, value16);
#if 0
			odm_set_bb_reg(dm, ODM_REG_TRMUX_11AC, BIT(21) | BIT20 | BIT19,
				       default_ant);	 /* Default RX */
			odm_set_bb_reg(dm, ODM_REG_TRMUX_11AC, BIT(24) | BIT23 | BIT22,
				       optional_ant); /* Optional RX */
			odm_set_bb_reg(dm, ODM_REG_TRMUX_11AC, BIT(27) | BIT26 | BIT25,
				       default_ant);	 /* Default TX */
#endif
		}

		if (dm->support_ic_type & (ODM_RTL8821C | ODM_RTL8822B | ODM_RTL8814A)) {
			odm_set_mac_reg(dm, 0x6D8, 0x7, default_tx_ant);		/*PathA Resp Tx*/
			/**/
		} else if (dm->support_ic_type == ODM_RTL8188E) {
			odm_set_mac_reg(dm, 0x6D8, BIT(7) | BIT(6),
					default_tx_ant);		/*PathA Resp Tx*/
			/**/
		} else {
			odm_set_mac_reg(dm, 0x6D8, BIT(10) | BIT(9) | BIT(8),
					default_tx_ant);	/*PathA Resp Tx*/
			/**/
		}

	} else { /* fat_tab->rx_idle_ant == ant */
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ Stay in Ori-ant ]  rx_idle_ant =%s\n",
			  (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		fat_tab->rx_idle_ant = ant;
	}
}

void phydm_set_antdiv_val(
	void			*dm_void,
	u32			*val_buf,
	u8			val_len
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;

	if (val_len != 1) {
		PHYDM_DBG(dm, ODM_COMP_API, "[Error][antdiv]Need val_len=1\n");
		return;
	}

	odm_update_rx_idle_ant(dm, (u8)(*val_buf));
}

void odm_update_tx_ant(
	void		*dm_void,
	u8		ant,
	u32		mac_id
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;
	u8	tx_ant;

	if (fat_tab->b_fix_tx_ant != NO_FIX_TX_ANT)
		ant = (fat_tab->b_fix_tx_ant == FIX_TX_AT_MAIN) ? MAIN_ANT : AUX_ANT;

	if (dm->ant_div_type == CG_TRX_SMART_ANTDIV)
		tx_ant = ant;
	else {
		if (ant == MAIN_ANT)
			tx_ant = ANT1_2G;
		else
			tx_ant = ANT2_2G;
	}

	fat_tab->antsel_a[mac_id] = tx_ant & BIT(0);
	fat_tab->antsel_b[mac_id] = (tx_ant & BIT(1)) >> 1;
	fat_tab->antsel_c[mac_id] = (tx_ant & BIT(2)) >> 2;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[Set TX-DESC value]: mac_id:(( %d )),  tx_ant = (( %s ))\n", mac_id,
		  (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	/* PHYDM_DBG(dm,DBG_ANT_DIV,"antsel_tr_mux=(( 3'b%d%d%d ))\n",fat_tab->antsel_c[mac_id] , fat_tab->antsel_b[mac_id] , fat_tab->antsel_a[mac_id] ); */

}

#ifdef BEAMFORMING_SUPPORT
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

void odm_bdc_init(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct _BF_DIV_COEX_	*dm_bdc_table = &dm->dm_bdc_table;

	PHYDM_DBG(dm, DBG_ANT_DIV, "\n[ BDC Initialization......]\n");
	dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
	dm_bdc_table->bdc_mode = BDC_MODE_NULL;
	dm_bdc_table->bdc_try_flag = 0;
	dm_bdc_table->bd_ccoex_type_wbfer = 0;
	dm->bdc_holdstate = 0xff;

	if (dm->support_ic_type == ODM_RTL8192E) {
		odm_set_bb_reg(dm, 0xd7c, 0x0FFFFFFF, 0x1081008);
		odm_set_bb_reg(dm, 0xd80, 0x0FFFFFFF, 0);
	} else if (dm->support_ic_type == ODM_RTL8812) {
		odm_set_bb_reg(dm, 0x9b0, 0x0FFFFFFF,
			       0x1081008);     /* 0x9b0[30:0] = 01081008 */
		odm_set_bb_reg(dm, 0x9b4, 0x0FFFFFFF,
			       0);                 /* 0x9b4[31:0] = 00000000 */
	}

}


void odm_CSI_on_off(
	void		*dm_void,
	u8			CSI_en
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	if (CSI_en == CSI_ON) {
		if (dm->support_ic_type == ODM_RTL8192E)
			odm_set_mac_reg(dm, 0xd84, BIT(11), 1);  /* 0xd84[11]=1 */
		else if (dm->support_ic_type == ODM_RTL8812)
			odm_set_mac_reg(dm, 0x9b0, BIT(31), 1);  /* 0x9b0[31]=1 */

	} else if (CSI_en == CSI_OFF) {
		if (dm->support_ic_type == ODM_RTL8192E)
			odm_set_mac_reg(dm, 0xd84, BIT(11), 0);  /* 0xd84[11]=0 */
		else if (dm->support_ic_type == ODM_RTL8812)
			odm_set_mac_reg(dm, 0x9b0, BIT(31), 0);  /* 0x9b0[31]=0 */
	}
}

void odm_bd_ccoex_type_with_bfer_client(
	void		*dm_void,
	u8			swch
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct _BF_DIV_COEX_	*dm_bdc_table = &dm->dm_bdc_table;
	u8     bd_ccoex_type_wbfer;

	if (swch == DIVON_CSIOFF) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "[BDCcoexType: 1] {DIV,CSI} ={1,0}\n");
		bd_ccoex_type_wbfer = 1;

		if (bd_ccoex_type_wbfer != dm_bdc_table->bd_ccoex_type_wbfer) {
			odm_ant_div_on_off(dm, ANTDIV_ON);
			odm_CSI_on_off(dm, CSI_OFF);
			dm_bdc_table->bd_ccoex_type_wbfer = 1;
		}
	} else if (swch == DIVOFF_CSION) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "[BDCcoexType: 2] {DIV,CSI} ={0,1}\n");
		bd_ccoex_type_wbfer = 2;

		if (bd_ccoex_type_wbfer != dm_bdc_table->bd_ccoex_type_wbfer) {
			odm_ant_div_on_off(dm, ANTDIV_OFF);
			odm_CSI_on_off(dm, CSI_ON);
			dm_bdc_table->bd_ccoex_type_wbfer = 2;
		}
	}
}

void odm_bf_ant_div_mode_arbitration(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct _BF_DIV_COEX_			*dm_bdc_table = &dm->dm_bdc_table;
	u8			current_bdc_mode;

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	PHYDM_DBG(dm, DBG_ANT_DIV, "\n");

	/* 2 mode 1 */
	if ((dm_bdc_table->num_txbfee_client != 0)
	    && (dm_bdc_table->num_txbfer_client == 0)) {
		current_bdc_mode = BDC_MODE_1;

		if (current_bdc_mode != dm_bdc_table->bdc_mode) {
			dm_bdc_table->bdc_mode = BDC_MODE_1;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
			dm_bdc_table->bdc_rx_idle_update_counter = 1;
			PHYDM_DBG(dm, DBG_ANT_DIV, "Change to (( Mode1 ))\n");
		}

		PHYDM_DBG(dm, DBG_ANT_DIV, "[Antdiv + BF coextance mode] : (( Mode1 ))\n");
	}
	/* 2 mode 2 */
	else if ((dm_bdc_table->num_txbfee_client == 0)
		 && (dm_bdc_table->num_txbfer_client != 0)) {
		current_bdc_mode = BDC_MODE_2;

		if (current_bdc_mode != dm_bdc_table->bdc_mode) {
			dm_bdc_table->bdc_mode = BDC_MODE_2;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			dm_bdc_table->bdc_try_flag = 0;
			PHYDM_DBG(dm, DBG_ANT_DIV, "Change to (( Mode2 ))\n");

		}
		PHYDM_DBG(dm, DBG_ANT_DIV, "[Antdiv + BF coextance mode] : (( Mode2 ))\n");
	}
	/* 2 mode 3 */
	else if ((dm_bdc_table->num_txbfee_client != 0)
		 && (dm_bdc_table->num_txbfer_client != 0)) {
		current_bdc_mode = BDC_MODE_3;

		if (current_bdc_mode != dm_bdc_table->bdc_mode) {
			dm_bdc_table->bdc_mode = BDC_MODE_3;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->bdc_rx_idle_update_counter = 1;
			PHYDM_DBG(dm, DBG_ANT_DIV, "Change to (( Mode3 ))\n");
		}

		PHYDM_DBG(dm, DBG_ANT_DIV, "[Antdiv + BF coextance mode] : (( Mode3 ))\n");
	}
	/* 2 mode 4 */
	else if ((dm_bdc_table->num_txbfee_client == 0)
		 && (dm_bdc_table->num_txbfer_client == 0)) {
		current_bdc_mode = BDC_MODE_4;

		if (current_bdc_mode != dm_bdc_table->bdc_mode) {
			dm_bdc_table->bdc_mode = BDC_MODE_4;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
			PHYDM_DBG(dm, DBG_ANT_DIV, "Change to (( Mode4 ))\n");
		}

		PHYDM_DBG(dm, DBG_ANT_DIV, "[Antdiv + BF coextance mode] : (( Mode4 ))\n");
	}
#endif

}

void odm_div_train_state_setting(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct _BF_DIV_COEX_	*dm_bdc_table = &dm->dm_bdc_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "\n*****[S T A R T ]*****  [2-0. DIV_TRAIN_STATE]\n");
	dm_bdc_table->bdc_try_counter = 2;
	dm_bdc_table->bdc_try_flag = 1;
	dm_bdc_table->BDC_state = bdc_bfer_train_state;
	odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
}

void odm_bd_ccoex_bfee_rx_div_arbitration(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct _BF_DIV_COEX_    *dm_bdc_table = &dm->dm_bdc_table;
	boolean stop_bf_flag;
	u8	bdc_active_mode;


#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***{ num_BFee,  num_BFer, num_client}  = (( %d  ,  %d  ,  %d))\n",
		  dm_bdc_table->num_txbfee_client, dm_bdc_table->num_txbfer_client,
		  dm_bdc_table->num_client);
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***{ num_BF_tars,  num_DIV_tars }  = ((  %d  ,  %d ))\n",
		  dm_bdc_table->num_bf_tar, dm_bdc_table->num_div_tar);

	/* 2 [ MIB control ] */
	if (dm->bdc_holdstate == 2) {
		odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
		dm_bdc_table->BDC_state = BDC_BF_HOLD_STATE;
		PHYDM_DBG(dm, DBG_ANT_DIV, "Force in [ BF STATE]\n");
		return;
	} else if (dm->bdc_holdstate == 1) {
		dm_bdc_table->BDC_state = BDC_DIV_HOLD_STATE;
		odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
		PHYDM_DBG(dm, DBG_ANT_DIV, "Force in [ DIV STATE]\n");
		return;
	}

	/* ------------------------------------------------------------ */



	/* 2 mode 2 & 3 */
	if (dm_bdc_table->bdc_mode == BDC_MODE_2
	    || dm_bdc_table->bdc_mode == BDC_MODE_3) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "\n{ Try_flag,  Try_counter } = {  %d , %d  }\n",
			  dm_bdc_table->bdc_try_flag, dm_bdc_table->bdc_try_counter);
		PHYDM_DBG(dm, DBG_ANT_DIV, "BDCcoexType = (( %d ))\n\n",
			  dm_bdc_table->bd_ccoex_type_wbfer);

		/* All Client have Bfer-Cap------------------------------- */
		if (dm_bdc_table->num_txbfer_client ==
		    dm_bdc_table->num_client) { /* BFer STA Only?: yes */
			PHYDM_DBG(dm, DBG_ANT_DIV, "BFer STA only?  (( Yes ))\n");
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
			return;
		} else
			PHYDM_DBG(dm, DBG_ANT_DIV, "BFer STA only?  (( No ))\n");
		/*  */
		if (dm_bdc_table->is_all_bf_sta_idle == false
		    && dm_bdc_table->is_all_div_sta_idle == true) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "All DIV-STA are idle, but BF-STA not\n");
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->BDC_state = bdc_bfer_train_state;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
			return;
		} else if (dm_bdc_table->is_all_bf_sta_idle == true
			   && dm_bdc_table->is_all_div_sta_idle == false) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "All BF-STA are idle, but DIV-STA not\n");
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
			return;
		}

		/* Select active mode-------------------------------------- */
		if (dm_bdc_table->num_bf_tar == 0) { /* Selsect_1,  Selsect_2 */
			if (dm_bdc_table->num_div_tar == 0) { /* Selsect_3 */
				PHYDM_DBG(dm, DBG_ANT_DIV, "Select active mode (( 1 ))\n");
				dm_bdc_table->bdc_active_mode = 1;
			} else {
				PHYDM_DBG(dm, DBG_ANT_DIV, "Select active mode  (( 2 ))\n");
				dm_bdc_table->bdc_active_mode = 2;
			}
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
			return;
		} else { /* num_bf_tar > 0 */
			if (dm_bdc_table->num_div_tar == 0) { /* Selsect_3 */
				PHYDM_DBG(dm, DBG_ANT_DIV, "Select active mode (( 3 ))\n");
				dm_bdc_table->bdc_active_mode = 3;
				dm_bdc_table->bdc_try_flag = 0;
				dm_bdc_table->BDC_state = bdc_bfer_train_state;
				odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
				return;
			} else { /* Selsect_4 */
				bdc_active_mode = 4;
				PHYDM_DBG(dm, DBG_ANT_DIV, "Select active mode (( 4 ))\n");

				if (bdc_active_mode != dm_bdc_table->bdc_active_mode) {
					dm_bdc_table->bdc_active_mode = 4;
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "Change to active mode (( 4 ))  &  return!!!\n");
					return;
				}
			}
		}

#if 1
		if (dm->bdc_holdstate == 0xff) {
			dm_bdc_table->BDC_state = BDC_DIV_HOLD_STATE;
			odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
			PHYDM_DBG(dm, DBG_ANT_DIV, "Force in [ DIV STATE]\n");
			return;
		}
#endif

		/* Does Client number changed ? ------------------------------- */
		if (dm_bdc_table->num_client != dm_bdc_table->pre_num_client) {
			dm_bdc_table->bdc_try_flag = 0;
			dm_bdc_table->BDC_state = BDC_DIV_TRAIN_STATE;
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[  The number of client has been changed !!!]   return to (( BDC_DIV_TRAIN_STATE ))\n");
		}
		dm_bdc_table->pre_num_client = dm_bdc_table->num_client;

		if (dm_bdc_table->bdc_try_flag == 0) {
			/* 2 DIV_TRAIN_STATE (mode 2-0) */
			if (dm_bdc_table->BDC_state == BDC_DIV_TRAIN_STATE)
				odm_div_train_state_setting(dm);
			/* 2 BFer_TRAIN_STATE (mode 2-1) */
			else if (dm_bdc_table->BDC_state == bdc_bfer_train_state) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "*****[2-1. BFer_TRAIN_STATE ]*****\n");

				/* if(dm_bdc_table->num_bf_tar==0) */
				/* { */
				/*	PHYDM_DBG(dm,DBG_ANT_DIV, "BF_tars exist?  : (( No )),   [ bdc_bfer_train_state ] >> [BDC_DIV_TRAIN_STATE]\n"); */
				/*	odm_div_train_state_setting( dm); */
				/* } */
				/* else */ /* num_bf_tar != 0 */
				/* { */
				dm_bdc_table->bdc_try_counter = 2;
				dm_bdc_table->bdc_try_flag = 1;
				dm_bdc_table->BDC_state = BDC_DECISION_STATE;
				odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "BF_tars exist?  : (( Yes )),   [ bdc_bfer_train_state ] >> [BDC_DECISION_STATE]\n");
				/* } */
			}
			/* 2 DECISION_STATE (mode 2-2) */
			else if (dm_bdc_table->BDC_state == BDC_DECISION_STATE) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "*****[2-2. DECISION_STATE]*****\n");
				/* if(dm_bdc_table->num_bf_tar==0) */
				/* { */
				/*	ODM_AntDiv_Printk(("BF_tars exist?  : (( No )),   [ DECISION_STATE ] >> [BDC_DIV_TRAIN_STATE]\n")); */
				/*	odm_div_train_state_setting( dm); */
				/* } */
				/* else */ /* num_bf_tar != 0 */
				/* { */
				if (dm_bdc_table->BF_pass == false || dm_bdc_table->DIV_pass == false)
					stop_bf_flag = true;
				else
					stop_bf_flag = false;

				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "BF_tars exist?  : (( Yes )),  {BF_pass, DIV_pass, stop_bf_flag }  = { %d, %d, %d }\n",
					  dm_bdc_table->BF_pass, dm_bdc_table->DIV_pass, stop_bf_flag);

				if (stop_bf_flag == true) { /* DIV_en */
					dm_bdc_table->bdc_hold_counter = 10; /* 20 */
					odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
					dm_bdc_table->BDC_state = BDC_DIV_HOLD_STATE;
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ stop_bf_flag= ((true)),   BDC_DECISION_STATE ] >> [BDC_DIV_HOLD_STATE]\n");
				} else { /* BF_en */
					dm_bdc_table->bdc_hold_counter = 10; /* 20 */
					odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
					dm_bdc_table->BDC_state = BDC_BF_HOLD_STATE;
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[stop_bf_flag= ((false)),   BDC_DECISION_STATE ] >> [BDC_BF_HOLD_STATE]\n");
				}
				/* } */
			}
			/* 2 BF-HOLD_STATE (mode 2-3) */
			else if (dm_bdc_table->BDC_state == BDC_BF_HOLD_STATE) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "*****[2-3. BF_HOLD_STATE ]*****\n");

				PHYDM_DBG(dm, DBG_ANT_DIV, "bdc_hold_counter = (( %d ))\n",
					  dm_bdc_table->bdc_hold_counter);

				if (dm_bdc_table->bdc_hold_counter == 1) {
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ BDC_BF_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE]\n");
					odm_div_train_state_setting(dm);
				} else {
					dm_bdc_table->bdc_hold_counter--;

					/* if(dm_bdc_table->num_bf_tar==0) */
					/* { */
					/*	PHYDM_DBG(dm,DBG_ANT_DIV, "BF_tars exist?  : (( No )),   [ BDC_BF_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE]\n"); */
					/*	odm_div_train_state_setting( dm); */
					/* } */
					/* else */ /* num_bf_tar != 0 */
					/* { */
					/* PHYDM_DBG(dm,DBG_ANT_DIV, "BF_tars exist?  : (( Yes ))\n"); */
					dm_bdc_table->BDC_state = BDC_BF_HOLD_STATE;
					odm_bd_ccoex_type_with_bfer_client(dm, DIVOFF_CSION);
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ BDC_BF_HOLD_STATE ] >> [BDC_BF_HOLD_STATE]\n");
					/* } */
				}

			}
			/* 2 DIV-HOLD_STATE (mode 2-4) */
			else if (dm_bdc_table->BDC_state == BDC_DIV_HOLD_STATE) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "*****[2-4. DIV_HOLD_STATE ]*****\n");

				PHYDM_DBG(dm, DBG_ANT_DIV, "bdc_hold_counter = (( %d ))\n",
					  dm_bdc_table->bdc_hold_counter);

				if (dm_bdc_table->bdc_hold_counter == 1) {
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ BDC_DIV_HOLD_STATE ] >> [BDC_DIV_TRAIN_STATE]\n");
					odm_div_train_state_setting(dm);
				} else {
					dm_bdc_table->bdc_hold_counter--;
					dm_bdc_table->BDC_state = BDC_DIV_HOLD_STATE;
					odm_bd_ccoex_type_with_bfer_client(dm, DIVON_CSIOFF);
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[ BDC_DIV_HOLD_STATE ] >> [BDC_DIV_HOLD_STATE]\n");
				}

			}

		} else if (dm_bdc_table->bdc_try_flag == 1) {
			/* 2 Set Training counter */
			if (dm_bdc_table->bdc_try_counter > 1) {
				dm_bdc_table->bdc_try_counter--;
				if (dm_bdc_table->bdc_try_counter == 1)
					dm_bdc_table->bdc_try_flag = 0;

				PHYDM_DBG(dm, DBG_ANT_DIV, "Training !!\n");
				/* return ; */
			}

		}

	}

	PHYDM_DBG(dm, DBG_ANT_DIV, "\n[end]\n");

#endif /* #if(DM_ODM_SUPPORT_TYPE  == ODM_AP) */






}

#endif
#endif /* #ifdef BEAMFORMING_SUPPORT */


#if (RTL8188E_SUPPORT == 1)


void odm_rx_hw_ant_div_init_88e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	value32;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

#if 0
	if (*(dm->mp_mode) == true) {
		odm_set_bb_reg(dm, ODM_REG_IGI_A_11N, BIT(7), 0); /* disable HW AntDiv */
		odm_set_bb_reg(dm, ODM_REG_LNA_SWITCH_11N, BIT(31), 1);  /* 1:CG, 0:CS */
		return;
	}
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8188E AntDiv_Init =>  ant_div_type=[CGCS_RX_HW_ANTDIV]\n");

	/* MAC setting */
	value32 = odm_get_mac_reg(dm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD);
	odm_set_mac_reg(dm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD,
			value32 | (BIT(23) | BIT(
					   25))); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	odm_set_bb_reg(dm, ODM_REG_PIN_CTRL_11N, BIT(9) | BIT(8),
		       0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(dm, ODM_REG_RX_ANT_CTRL_11N, BIT(10),
		       0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(dm, ODM_REG_LNA_SWITCH_11N, BIT(22),
		       1);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(dm, ODM_REG_LNA_SWITCH_11N, BIT(31),
		       1);	/* regb2c[31]=1'b1	 */ /* output at CG only */
	/* OFDM Settings */
	odm_set_bb_reg(dm, ODM_REG_ANTDIV_PARA1_11N, MASKDWORD, 0x000000a0);
	/* CCK Settings */
	odm_set_bb_reg(dm, ODM_REG_BB_PWR_SAV4_11N, BIT(7),
		       1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(dm, ODM_REG_CCK_ANTDIV_PARA2_11N, BIT(4),
		       1); /* CCK complete HW AntDiv within 64 samples */

	odm_set_bb_reg(dm, ODM_REG_ANT_MAPPING1_11N, 0xFFFF,
		       0x0001);	/* antenna mapping table */

	fat_tab->enable_ctrl_frame_antdiv = 1;
}

void odm_trx_hw_ant_div_init_88e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	value32;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

#if 0
	if (*(dm->mp_mode) == true) {
		odm_set_bb_reg(dm, ODM_REG_IGI_A_11N, BIT(7), 0); /* disable HW AntDiv */
		odm_set_bb_reg(dm, ODM_REG_RX_ANT_CTRL_11N, BIT(5) | BIT4 | BIT3,
			       0); /* Default RX   (0/1) */
		return;
	}
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8188E AntDiv_Init =>  ant_div_type=[CG_TRX_HW_ANTDIV (SPDT)]\n");

	/* MAC setting */
	value32 = odm_get_mac_reg(dm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD);
	odm_set_mac_reg(dm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD,
			value32 | (BIT(23) | BIT(
					   25))); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	odm_set_bb_reg(dm, ODM_REG_PIN_CTRL_11N, BIT(9) | BIT(8),
		       0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(dm, ODM_REG_RX_ANT_CTRL_11N, BIT(10),
		       0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(dm, ODM_REG_LNA_SWITCH_11N, BIT(22),
		       0);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(dm, ODM_REG_LNA_SWITCH_11N, BIT(31),
		       1);	/* regb2c[31]=1'b1	 */ /* output at CG only */
	/* OFDM Settings */
	odm_set_bb_reg(dm, ODM_REG_ANTDIV_PARA1_11N, MASKDWORD, 0x000000a0);
	/* CCK Settings */
	odm_set_bb_reg(dm, ODM_REG_BB_PWR_SAV4_11N, BIT(7),
		       1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(dm, ODM_REG_CCK_ANTDIV_PARA2_11N, BIT(4),
		       1); /* CCK complete HW AntDiv within 64 samples */

	/* antenna mapping table */
	if (!dm->is_mp_chip) { /* testchip */
		odm_set_bb_reg(dm, ODM_REG_RX_DEFAULT_A_11N, BIT(10) | BIT(9) | BIT(8),
			       1);	/* Reg858[10:8]=3'b001 */
		odm_set_bb_reg(dm, ODM_REG_RX_DEFAULT_A_11N, BIT(13) | BIT(12) | BIT(11),
			       2);	/* Reg858[13:11]=3'b010 */
	} else /* MPchip */
		odm_set_bb_reg(dm, ODM_REG_ANT_MAPPING1_11N, MASKDWORD,
			       0x0201);	/*Reg914=3'b010, Reg915=3'b001*/

	fat_tab->enable_ctrl_frame_antdiv = 1;
}


#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
void odm_smart_hw_ant_div_init_88e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	value32, i;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8188E AntDiv_Init =>  ant_div_type=[CG_TRX_SMART_ANTDIV]\n");

#if 0
	if (*(dm->mp_mode) == true) {
		PHYDM_DBG(dm, ODM_COMP_INIT, "dm->ant_div_type: %d\n", dm->ant_div_type);
		return;
	}
#endif

	fat_tab->train_idx = 0;
	fat_tab->fat_state = FAT_PREPARE_STATE;

	dm->fat_comb_a = 5;
	dm->antdiv_intvl = 0x64; /* 100ms */

	for (i = 0; i < 6; i++)
		fat_tab->bssid[i] = 0;
	for (i = 0; i < (dm->fat_comb_a) ; i++) {
		fat_tab->ant_sum_rssi[i] = 0;
		fat_tab->ant_rssi_cnt[i] = 0;
		fat_tab->ant_ave_rssi[i] = 0;
	}

	/* MAC setting */
	value32 = odm_get_mac_reg(dm, 0x4c, MASKDWORD);
	odm_set_mac_reg(dm, 0x4c, MASKDWORD,
			value32 | (BIT(23) | BIT(25))); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	value32 = odm_get_mac_reg(dm,  0x7B4, MASKDWORD);
	odm_set_mac_reg(dm, 0x7b4, MASKDWORD,
			value32 | (BIT(16) | BIT(17))); /* Reg7B4[16]=1 enable antenna training, Reg7B4[17]=1 enable A2 match */
	/* value32 = platform_efio_read_4byte(adapter, 0x7B4); */
	/* platform_efio_write_4byte(adapter, 0x7b4, value32|BIT(18));	 */ /* append MACID in reponse packet */

	/* Match MAC ADDR */
	odm_set_mac_reg(dm, 0x7b4, 0xFFFF, 0);
	odm_set_mac_reg(dm, 0x7b0, MASKDWORD, 0);

	odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8),
		       0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(dm, 0x864, BIT(10),
		       0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(dm, 0xb2c, BIT(22),
		       0);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(dm, 0xb2c, BIT(31),
		       0);	/* regb2c[31]=1'b1	 */ /* output at CS only */
	odm_set_bb_reg(dm, 0xca4, MASKDWORD, 0x000000a0);

	/* antenna mapping table */
	if (dm->fat_comb_a == 2) {
		if (!dm->is_mp_chip) { /* testchip */
			odm_set_bb_reg(dm, 0x858, BIT(10) | BIT(9) | BIT(8),
				       1);	/* Reg858[10:8]=3'b001 */
			odm_set_bb_reg(dm, 0x858, BIT(13) | BIT(12) | BIT(11),
				       2);	/* Reg858[13:11]=3'b010 */
		} else { /* MPchip */
			odm_set_bb_reg(dm, 0x914, MASKBYTE0, 1);
			odm_set_bb_reg(dm, 0x914, MASKBYTE1, 2);
		}
	} else {
		if (!dm->is_mp_chip) { /* testchip */
			odm_set_bb_reg(dm, 0x858, BIT(10) | BIT(9) | BIT(8),
				       0);	/* Reg858[10:8]=3'b000 */
			odm_set_bb_reg(dm, 0x858, BIT(13) | BIT(12) | BIT(11),
				       1);	/* Reg858[13:11]=3'b001 */
			odm_set_bb_reg(dm, 0x878, BIT(16), 0);
			odm_set_bb_reg(dm, 0x858, BIT(15) | BIT(14),
				       2);	/* (Reg878[0],Reg858[14:15])=3'b010 */
			odm_set_bb_reg(dm, 0x878, BIT(19) | BIT(18) | BIT(17),
				       3); /* Reg878[3:1]=3b'011 */
			odm_set_bb_reg(dm, 0x878, BIT(22) | BIT(21) | BIT(20),
				       4); /* Reg878[6:4]=3b'100 */
			odm_set_bb_reg(dm, 0x878, BIT(25) | BIT(24) | BIT(23),
				       5); /* Reg878[9:7]=3b'101 */
			odm_set_bb_reg(dm, 0x878, BIT(28) | BIT(27) | BIT(26),
				       6); /* Reg878[12:10]=3b'110 */
			odm_set_bb_reg(dm, 0x878, BIT(31) | BIT(30) | BIT(29),
				       7); /* Reg878[15:13]=3b'111 */
		} else { /* MPchip */
			odm_set_bb_reg(dm, 0x914, MASKBYTE0, 4);     /* 0: 3b'000 */
			odm_set_bb_reg(dm, 0x914, MASKBYTE1, 2);     /* 1: 3b'001 */
			odm_set_bb_reg(dm, 0x914, MASKBYTE2, 0);     /* 2: 3b'010 */
			odm_set_bb_reg(dm, 0x914, MASKBYTE3, 1);     /* 3: 3b'011 */
			odm_set_bb_reg(dm, 0x918, MASKBYTE0, 3);     /* 4: 3b'100 */
			odm_set_bb_reg(dm, 0x918, MASKBYTE1, 5);     /* 5: 3b'101 */
			odm_set_bb_reg(dm, 0x918, MASKBYTE2, 6);     /* 6: 3b'110 */
			odm_set_bb_reg(dm, 0x918, MASKBYTE3, 255); /* 7: 3b'111 */
		}
	}

	/* Default ant setting when no fast training */
	odm_set_bb_reg(dm, 0x864, BIT(5) | BIT(4) | BIT(3), 0);	/* Default RX */
	odm_set_bb_reg(dm, 0x864, BIT(8) | BIT(7) | BIT(6), 1);	/* Optional RX */
	odm_set_bb_reg(dm, 0x860, BIT(14) | BIT(13) | BIT(12), 0); /* Default TX */

	/* Enter Traing state */
	odm_set_bb_reg(dm, 0x864, BIT(2) | BIT(1) | BIT(0),
		       (dm->fat_comb_a -
			1));	/* reg864[2:0]=3'd6	 */ /* ant combination=reg864[2:0]+1 */

	/* SW Control */
	/* phy_set_bb_reg(adapter, 0x864, BIT10, 1); */
	/* phy_set_bb_reg(adapter, 0x870, BIT9, 1); */
	/* phy_set_bb_reg(adapter, 0x870, BIT8, 1); */
	/* phy_set_bb_reg(adapter, 0x864, BIT11, 1); */
	/* phy_set_bb_reg(adapter, 0x860, BIT9, 0); */
	/* phy_set_bb_reg(adapter, 0x860, BIT8, 0); */
}
#endif

#endif /* #if (RTL8188E_SUPPORT == 1) */


#if (RTL8192E_SUPPORT == 1)
void odm_rx_hw_ant_div_init_92e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

#if 0
	if (*(dm->mp_mode) == true) {
		odm_ant_div_on_off(dm, ANTDIV_OFF);
		odm_set_bb_reg(dm, 0xc50, BIT(8),
			       0); /* r_rxdiv_enable_anta  regc50[8]=1'b0  0: control by c50[9] */
		odm_set_bb_reg(dm, 0xc50, BIT(9), 1);  /* 1:CG, 0:CS */
		return;
	}
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8192E AntDiv_Init =>  ant_div_type=[CGCS_RX_HW_ANTDIV]\n");

	/* Pin Settings */
	odm_set_bb_reg(dm, 0x870, BIT(8),
		       0);/* reg870[8]=1'b0,     */ /* "antsel" is controled by HWs */
	odm_set_bb_reg(dm, 0xc50, BIT(8),
		       1); /* regc50[8]=1'b1   */ /* " CS/CG switching" is controled by HWs */

	/* Mapping table */
	odm_set_bb_reg(dm, 0x914, 0xFFFF, 0x0100); /* antenna mapping table */

	/* OFDM Settings */
	odm_set_bb_reg(dm, 0xca4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0xca4, 0x7FF000, 0x0); /* bias */

	/* CCK Settings */
	odm_set_bb_reg(dm, 0xa04, 0xF000000,
		       0); /* Select which path to receive for CCK_1 & CCK_2 */
	odm_set_bb_reg(dm, 0xb34, BIT(30),
		       0); /* (92E) ANTSEL_CCK_opt = r_en_antsel_cck? ANTSEL_CCK: 1'b0 */
	odm_set_bb_reg(dm, 0xa74, BIT(7), 1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(dm, 0xa0c, BIT(4),
		       1); /* CCK complete HW AntDiv within 64 samples */

#ifdef ODM_EVM_ENHANCE_ANTDIV
	phydm_evm_sw_antdiv_init(dm);
#endif

}

void odm_trx_hw_ant_div_init_92e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

#if 0
	if (*(dm->mp_mode) == true) {
		odm_ant_div_on_off(dm, ANTDIV_OFF);
		odm_set_bb_reg(dm, 0xc50, BIT(8),
			       0); /* r_rxdiv_enable_anta  regc50[8]=1'b0  0: control by c50[9] */
		odm_set_bb_reg(dm, 0xc50, BIT(9), 1);  /* 1:CG, 0:CS */
		return;
	}
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8192E AntDiv_Init =>  ant_div_type=[ Only for DIR605, CG_TRX_HW_ANTDIV]\n");

	/* 3 --RFE pin setting--------- */
	/* [MAC] */
	odm_set_mac_reg(dm, 0x38, BIT(11),
			1);            /* DBG PAD Driving control (GPIO 8) */
	odm_set_mac_reg(dm, 0x4c, BIT(23), 0);            /* path-A, RFE_CTRL_3 */
	odm_set_mac_reg(dm, 0x4c, BIT(29), 1);            /* path-A, RFE_CTRL_8 */
	/* [BB] */
	odm_set_bb_reg(dm, 0x944, BIT(3), 1);              /* RFE_buffer */
	odm_set_bb_reg(dm, 0x944, BIT(8), 1);
	odm_set_bb_reg(dm, 0x940, BIT(7) | BIT(6),
		       0x0); /* r_rfe_path_sel_   (RFE_CTRL_3) */
	odm_set_bb_reg(dm, 0x940, BIT(17) | BIT(16),
		       0x0); /* r_rfe_path_sel_   (RFE_CTRL_8) */
	odm_set_bb_reg(dm, 0x944, BIT(31), 0);     /* RFE_buffer */
	odm_set_bb_reg(dm, 0x92C, BIT(3), 0);     /* rfe_inv  (RFE_CTRL_3) */
	odm_set_bb_reg(dm, 0x92C, BIT(8), 1);     /* rfe_inv  (RFE_CTRL_8) */
	odm_set_bb_reg(dm, 0x930, 0xF000, 0x8);           /* path-A, RFE_CTRL_3 */
	odm_set_bb_reg(dm, 0x934, 0xF, 0x8);           /* path-A, RFE_CTRL_8 */
	/* 3 ------------------------- */

	/* Pin Settings */
	odm_set_bb_reg(dm, 0xC50, BIT(8),
		       0);	/* path-A  	 */ /* disable CS/CG switch */

#if 0
	/* Let it follows PHY_REG for bit9 setting */
	if (dm->priv->pshare->rf_ft_var.use_ext_pa
	    || dm->priv->pshare->rf_ft_var.use_ext_lna)
		odm_set_bb_reg(dm, 0xC50, BIT(9), 1);	/* path-A 	output at CS */
	else
		odm_set_bb_reg(dm, 0xC50, BIT(9),
			       0);	/* path-A 	output at CG ->normal power */
#endif

	odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8),
		       0);	/* path-A*/	/* antsel antselb by HW */
	odm_set_bb_reg(dm, 0xB38, BIT(10), 0);	/* path-A	*/	/* antsel2 by HW */

	/* Mapping table */
	odm_set_bb_reg(dm, 0x914, 0xFFFF, 0x0100); /* antenna mapping table */

	/* OFDM Settings */
	odm_set_bb_reg(dm, 0xca4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0xca4, 0x7FF000, 0x0); /* bias */

	/* CCK Settings */
	odm_set_bb_reg(dm, 0xa04, 0xF000000,
		       0); /* Select which path to receive for CCK_1 & CCK_2 */
	odm_set_bb_reg(dm, 0xb34, BIT(30),
		       0); /* (92E) ANTSEL_CCK_opt = r_en_antsel_cck? ANTSEL_CCK: 1'b0 */
	odm_set_bb_reg(dm, 0xa74, BIT(7), 1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(dm, 0xa0c, BIT(4),
		       1); /* CCK complete HW AntDiv within 64 samples */

#ifdef ODM_EVM_ENHANCE_ANTDIV
	phydm_evm_sw_antdiv_init(dm);
#endif
}

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
void odm_smart_hw_ant_div_init_92e(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8192E AntDiv_Init =>  ant_div_type=[CG_TRX_SMART_ANTDIV]\n");
}
#endif

#endif /* #if (RTL8192E_SUPPORT == 1) */

#if (RTL8723D_SUPPORT == 1)
void odm_trx_hw_ant_div_init_8723d(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[8723D] AntDiv_Init =>  ant_div_type=[S0S1_HW_TRX_AntDiv]\n");

	/*BT Coexistence*/
	/*keep antsel_map when GNT_BT = 1*/
	odm_set_bb_reg(dm, 0x864, BIT(12), 1);
	/* Disable hw antsw & fast_train.antsw when GNT_BT=1 */
	odm_set_bb_reg(dm, 0x874, BIT(23), 0);
	/* Disable hw antsw & fast_train.antsw when BT TX/RX */
	odm_set_bb_reg(dm, 0xE64, 0xFFFF0000, 0x000c);


	odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8), 0);
	/*PTA setting: WL_BB_SEL_BTG_TRXG_anta,  (1: HW CTRL  0: SW CTRL)*/
	/*odm_set_bb_reg(dm, 0x948, BIT6, 0);*/
	/*odm_set_bb_reg(dm, 0x948, BIT8, 0);*/
	/*GNT_WL tx*/
	odm_set_bb_reg(dm, 0x950, BIT(29), 0);


	/*Mapping Table*/
	odm_set_bb_reg(dm, 0x914, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0x914, MASKBYTE1, 3);
	/* odm_set_bb_reg(dm, 0x864, BIT5|BIT4|BIT3, 0); */
	/* odm_set_bb_reg(dm, 0x864, BIT8|BIT7|BIT6, 1); */

	/* Set WLBB_SEL_RF_ON 1 if RXFIR_PWDB > 0xCcc[3:0] */
	odm_set_bb_reg(dm, 0xCcc, BIT(12), 0);
	/* Low-to-High threshold for WLBB_SEL_RF_ON when OFDM enable */
	odm_set_bb_reg(dm, 0xCcc, 0x0F, 0x01);
	/* High-to-Low threshold for WLBB_SEL_RF_ON when OFDM enable */
	odm_set_bb_reg(dm, 0xCcc, 0xF0, 0x0);
	/* b Low-to-High threshold for WLBB_SEL_RF_ON when OFDM disable ( only CCK ) */
	odm_set_bb_reg(dm, 0xAbc, 0xFF, 0x06);
	/* High-to-Low threshold for WLBB_SEL_RF_ON when OFDM disable ( only CCK ) */
	odm_set_bb_reg(dm, 0xAbc, 0xFF00, 0x00);


	/*OFDM HW AntDiv Parameters*/
	odm_set_bb_reg(dm, 0xCA4, 0x7FF, 0xa0);
	odm_set_bb_reg(dm, 0xCA4, 0x7FF000, 0x00);
	odm_set_bb_reg(dm, 0xC5C, BIT(20) | BIT(19) | BIT(18), 0x04);

	/*CCK HW AntDiv Parameters*/
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1);
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1);
	odm_set_bb_reg(dm, 0xAA8, BIT(8), 0);

	odm_set_bb_reg(dm, 0xA0C, 0x0F, 0xf);
	odm_set_bb_reg(dm, 0xA14, 0x1F, 0x8);
	odm_set_bb_reg(dm, 0xA10, BIT(13), 0x1);
	odm_set_bb_reg(dm, 0xA74, BIT(8), 0x0);
	odm_set_bb_reg(dm, 0xB34, BIT(30), 0x1);

	/*disable antenna training	*/
	odm_set_bb_reg(dm, 0xE08, BIT(16), 0);
	odm_set_bb_reg(dm, 0xc50, BIT(8), 0);

}
/*Mingzhi 2017-05-08*/

void odm_update_rx_idle_ant_8723d(
	void			*dm_void,
	u8			ant,
	u32			default_ant,
	u32			optional_ant
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	void		*adapter = dm->adapter;
	u8			count = 0;


	/*	odm_set_bb_reg(dm, 0x948, BIT(6), 0x1);	*/
	odm_set_bb_reg(dm, 0x948, BIT(7), default_ant);
	odm_set_bb_reg(dm, 0x864, BIT(5) | BIT(4) | BIT(3),
		       default_ant);      /*Default RX*/
	odm_set_bb_reg(dm, 0x864, BIT(8) | BIT(7) | BIT(6),
		       optional_ant);     /*Optional RX*/
	odm_set_bb_reg(dm, 0x860, BIT(14) | BIT(13) | BIT(12),
		       default_ant);    /*Default TX*/
	fat_tab->rx_idle_ant = ant;

}

void phydm_set_tx_ant_pwr_8723d(
	void			*dm_void,
	u8			ant
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	PADAPTER    adapter = (PADAPTER)dm->adapter;

	fat_tab->rx_idle_ant = ant;

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	adapter->HalFunc.SetTxPowerLevelHandler(adapter, *dm->channel);
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
	rtw_hal_set_tx_power_level(adapter, *dm->channel);
#endif

}
#endif

#if (RTL8723B_SUPPORT == 1)
void odm_trx_hw_ant_div_init_8723b(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8723B AntDiv_Init =>  ant_div_type=[CG_TRX_HW_ANTDIV(DPDT)]\n");

	/* Mapping Table */
	odm_set_bb_reg(dm, 0x914, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0x914, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xCA4, 0x7FF, 0xa0); /* thershold */
	odm_set_bb_reg(dm, 0xCA4, 0x7FF000, 0x00); /* bias */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	/* BT Coexistence */
	odm_set_bb_reg(dm, 0x864, BIT(12),
		       0); /* keep antsel_map when GNT_BT = 1 */
	odm_set_bb_reg(dm, 0x874, BIT(23),
		       0); /* Disable hw antsw & fast_train.antsw when GNT_BT=1 */

	/* Output Pin Settings */
	odm_set_bb_reg(dm, 0x870, BIT(8), 0);

	odm_set_bb_reg(dm, 0x948, BIT(6),
		       0); /* WL_BB_SEL_BTG_TRXG_anta,  (1: HW CTRL  0: SW CTRL) */
	odm_set_bb_reg(dm, 0x948, BIT(7), 0);

	odm_set_mac_reg(dm, 0x40, BIT(3), 1);
	odm_set_mac_reg(dm, 0x38, BIT(11), 1);
	odm_set_mac_reg(dm, 0x4C,  BIT(24) | BIT(23),
			2); /* select DPDT_P and DPDT_N as output pin */

	odm_set_bb_reg(dm, 0x944, BIT(0) | BIT(1), 3); /* in/out */
	odm_set_bb_reg(dm, 0x944, BIT(31), 0);

	odm_set_bb_reg(dm, 0x92C, BIT(1), 0); /* DPDT_P non-inverse */
	odm_set_bb_reg(dm, 0x92C, BIT(0), 1); /* DPDT_N inverse */

	odm_set_bb_reg(dm, 0x930, 0xF0, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0x930, 0xF, 8); /* DPDT_N = ANTSEL[0] */

	/* 2 [--For HW Bug setting] */
	if (dm->ant_type == ODM_AUTO_ANT)
		odm_set_bb_reg(dm, 0xA00, BIT(15),
			       0); /* CCK AntDiv function block enable */

}



void odm_s0s1_sw_ant_div_init_8723b(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8723B AntDiv_Init => ant_div_type=[ S0S1_SW_AntDiv]\n");

	/* Mapping Table */
	odm_set_bb_reg(dm, 0x914, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0x914, MASKBYTE1, 1);

	/* Output Pin Settings */
	/* odm_set_bb_reg(dm, 0x948, BIT6, 0x1); */
	odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8), 0);

	fat_tab->is_become_linked  = false;
	dm_swat_table->try_flag = SWAW_STEP_INIT;
	dm_swat_table->double_chk_flag = 0;

	/* 2 [--For HW Bug setting] */
	odm_set_bb_reg(dm, 0x80C, BIT(21), 0); /* TX ant  by Reg */

}

void odm_update_rx_idle_ant_8723b(
	void			*dm_void,
	u8			ant,
	u32			default_ant,
	u32			optional_ant
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	PADAPTER    adapter = (PADAPTER)dm->adapter;
	u8			count = 0;
	/*u8			u1_temp;*/
	/*u8			h2c_parameter;*/

	if ((!dm->is_linked) && (dm->ant_type == ODM_AUTO_ANT)) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[ Update Rx-Idle-ant ] 8723B: Fail to set RX antenna due to no link\n");
		return;
	}

#if 0
	/* Send H2C command to FW */
	/* Enable wifi calibration */
	h2c_parameter = true;
	odm_fill_h2c_cmd(dm, ODM_H2C_WIFI_CALIBRATION, 1, &h2c_parameter);

	/* Check if H2C command sucess or not (0x1e6) */
	u1_temp = odm_read_1byte(dm, 0x1e6);
	while ((u1_temp != 0x1) && (count < 100)) {
		ODM_delay_us(10);
		u1_temp = odm_read_1byte(dm, 0x1e6);
		count++;
	}
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Update Rx-Idle-ant ] 8723B: H2C command status = %d, count = %d\n",
		  u1_temp, count);

	if (u1_temp == 0x1) {
		/* Check if BT is doing IQK (0x1e7) */
		count = 0;
		u1_temp = odm_read_1byte(dm, 0x1e7);
		while ((!(u1_temp & BIT(0)))  && (count < 100)) {
			ODM_delay_us(50);
			u1_temp = odm_read_1byte(dm, 0x1e7);
			count++;
		}
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[ Update Rx-Idle-ant ] 8723B: BT IQK status = %d, count = %d\n", u1_temp,
			  count);

		if (u1_temp & BIT(0)) {
			odm_set_bb_reg(dm, 0x948, BIT(6), 0x1);
			odm_set_bb_reg(dm, 0x948, BIT(9), default_ant);
			odm_set_bb_reg(dm, 0x864, BIT(5) | BIT4 | BIT3,
				       default_ant);	/* Default RX */
			odm_set_bb_reg(dm, 0x864, BIT(8) | BIT7 | BIT6,
				       optional_ant);	/* Optional RX */
			odm_set_bb_reg(dm, 0x860, BIT(14) | BIT13 | BIT12,
				       default_ant); /* Default TX */
			fat_tab->rx_idle_ant = ant;

			/* Set TX AGC by S0/S1 */
			/* Need to consider Linux driver */
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
			adapter->hal_func.set_tx_power_level_handler(adapter, *dm->channel);
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
			rtw_hal_set_tx_power_level(adapter, *dm->channel);
#endif

			/* Set IQC by S0/S1 */
			odm_set_iqc_by_rfpath(dm, default_ant);
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[ Update Rx-Idle-ant ] 8723B: Success to set RX antenna\n");
		} else
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[ Update Rx-Idle-ant ] 8723B: Fail to set RX antenna due to BT IQK\n");
	} else
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[ Update Rx-Idle-ant ] 8723B: Fail to set RX antenna due to H2C command fail\n");

	/* Send H2C command to FW */
	/* Disable wifi calibration */
	h2c_parameter = false;
	odm_fill_h2c_cmd(dm, ODM_H2C_WIFI_CALIBRATION, 1, &h2c_parameter);
#else

	odm_set_bb_reg(dm, 0x948, BIT(6), 0x1);
	odm_set_bb_reg(dm, 0x948, BIT(9), default_ant);
	odm_set_bb_reg(dm, 0x864, BIT(5) | BIT(4) | BIT(3),
		       default_ant);      /*Default RX*/
	odm_set_bb_reg(dm, 0x864, BIT(8) | BIT(7) | BIT(6),
		       optional_ant);     /*Optional RX*/
	odm_set_bb_reg(dm, 0x860, BIT(14) | BIT(13) | BIT(12),
		       default_ant);    /*Default TX*/
	fat_tab->rx_idle_ant = ant;

	/* Set TX AGC by S0/S1 */
	/* Need to consider Linux driver */
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	adapter->HalFunc.SetTxPowerLevelHandler(adapter, *dm->channel);
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
	rtw_hal_set_tx_power_level(adapter, *dm->channel);
#endif

	/* Set IQC by S0/S1 */
	odm_set_iqc_by_rfpath(dm, default_ant);
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Update Rx-Idle-ant ] 8723B: Success to set RX antenna\n");

#endif
}

boolean phydm_is_bt_enable_8723b(
	void			*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32			bt_state;
	/*u32			reg75;*/

	/*reg75 = odm_get_bb_reg(dm, 0x74, BIT8);*/
	/*odm_set_bb_reg(dm, 0x74, BIT8, 0x0);*/
	odm_set_bb_reg(dm, 0xa0, BIT(24) | BIT(25) | BIT(26), 0x5);
	bt_state = odm_get_bb_reg(dm, 0xa0, (BIT(3) | BIT(2) | BIT(1) | BIT(0)));
	/*odm_set_bb_reg(dm, 0x74, BIT8, reg75);*/

	if ((bt_state == 4) || (bt_state == 7) || (bt_state == 9)
	    || (bt_state == 13))
		return true;
	else
		return false;
}
#endif /* #if (RTL8723B_SUPPORT == 1) */

#if (RTL8821A_SUPPORT == 1)

void odm_trx_hw_ant_div_init_8821a(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8821A AntDiv_Init => ant_div_type=[ CG_TRX_HW_ANTDIV (DPDT)]\n");

	/* Output Pin Settings */
	odm_set_mac_reg(dm, 0x4C, BIT(25), 0);

	odm_set_mac_reg(dm, 0x64, BIT(29), 1); /* PAPE by WLAN control */
	odm_set_mac_reg(dm, 0x64, BIT(28), 1); /* LNAON by WLAN control */

	odm_set_bb_reg(dm, 0xCB8, BIT(16), 0);

	odm_set_mac_reg(dm, 0x4C, BIT(23),
			0); /* select DPDT_P and DPDT_N as output pin */
	odm_set_mac_reg(dm, 0x4C, BIT(24), 1); /* by WLAN control */
	odm_set_bb_reg(dm, 0xCB4, 0xF, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, 0xF0, 8); /* DPDT_N = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, BIT(29), 0); /* DPDT_P non-inverse */
	odm_set_bb_reg(dm, 0xCB4, BIT(28), 1); /* DPDT_N inverse */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x10); /* bias */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	odm_set_bb_reg(dm, 0x800, BIT(25),
		       0); /* ANTSEL_CCK sent to the smart_antenna circuit */
	odm_set_bb_reg(dm, 0xA00, BIT(15),
		       0); /* CCK AntDiv function block enable */

	/* BT Coexistence */
	odm_set_bb_reg(dm, 0xCAC, BIT(9), 1); /* keep antsel_map when GNT_BT = 1 */
	odm_set_bb_reg(dm, 0x804, BIT(4),
		       1); /* Disable hw antsw & fast_train.antsw when GNT_BT=1 */

	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* response TX ant by RX ant */
	odm_set_mac_reg(dm, 0x668, BIT(3), 1);

}

void odm_s0s1_sw_ant_div_init_8821a(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table = &dm->dm_swat_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8821A AntDiv_Init => ant_div_type=[ S0S1_SW_AntDiv]\n");

	/* Output Pin Settings */
	odm_set_mac_reg(dm, 0x4C, BIT(25), 0);

	odm_set_mac_reg(dm, 0x64, BIT(29), 1); /* PAPE by WLAN control */
	odm_set_mac_reg(dm, 0x64, BIT(28), 1); /* LNAON by WLAN control */

	odm_set_bb_reg(dm, 0xCB8, BIT(16), 0);

	odm_set_mac_reg(dm, 0x4C, BIT(23),
			0); /* select DPDT_P and DPDT_N as output pin */
	odm_set_mac_reg(dm, 0x4C, BIT(24), 1); /* by WLAN control */
	odm_set_bb_reg(dm, 0xCB4, 0xF, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, 0xF0, 8); /* DPDT_N = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, BIT(29), 0); /* DPDT_P non-inverse */
	odm_set_bb_reg(dm, 0xCB4, BIT(28), 1); /* DPDT_N inverse */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x10); /* bias */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	odm_set_bb_reg(dm, 0x800, BIT(25),
		       0); /* ANTSEL_CCK sent to the smart_antenna circuit */
	odm_set_bb_reg(dm, 0xA00, BIT(15),
		       0); /* CCK AntDiv function block enable */

	/* BT Coexistence */
	odm_set_bb_reg(dm, 0xCAC, BIT(9), 1); /* keep antsel_map when GNT_BT = 1 */
	odm_set_bb_reg(dm, 0x804, BIT(4),
		       1); /* Disable hw antsw & fast_train.antsw when GNT_BT=1 */

	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* response TX ant by RX ant */
	odm_set_mac_reg(dm, 0x668, BIT(3), 1);


	odm_set_bb_reg(dm, 0x900, BIT(18), 0);

	dm_swat_table->try_flag = SWAW_STEP_INIT;
	dm_swat_table->double_chk_flag = 0;
	dm_swat_table->cur_antenna = MAIN_ANT;
	dm_swat_table->pre_antenna = MAIN_ANT;
	dm_swat_table->swas_no_link_state = 0;

}
#endif /* #if (RTL8821A_SUPPORT == 1) */

#if (RTL8821C_SUPPORT == 1)
void odm_trx_hw_ant_div_init_8821c(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8821C AntDiv_Init => ant_div_type=[ CG_TRX_HW_ANTDIV (DPDT)]\n");
	/* Output Pin Settings */
	odm_set_mac_reg(dm, 0x4C, BIT(25), 0);

	odm_set_mac_reg(dm, 0x64, BIT(29), 1); /* PAPE by WLAN control */
	odm_set_mac_reg(dm, 0x64, BIT(28), 1); /* LNAON by WLAN control */

	odm_set_bb_reg(dm, 0xCB8, BIT(16), 0);

	odm_set_mac_reg(dm, 0x4C, BIT(23),
			0); /* select DPDT_P and DPDT_N as output pin */
	odm_set_mac_reg(dm, 0x4C, BIT(24), 1); /* by WLAN control */
	odm_set_bb_reg(dm, 0xCB4, 0xF, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, 0xF0, 8); /* DPDT_N = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, BIT(29), 0); /* DPDT_P non-inverse */
	odm_set_bb_reg(dm, 0xCB4, BIT(28), 1); /* DPDT_N inverse */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x10); /* bias */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	odm_set_bb_reg(dm, 0x800, BIT(25),
		       0); /* ANTSEL_CCK sent to the smart_antenna circuit */
	odm_set_bb_reg(dm, 0xA00, BIT(15),
		       0); /* CCK AntDiv function block enable */

	/* BT Coexistence */
	odm_set_bb_reg(dm, 0xCAC, BIT(9), 1); /* keep antsel_map when GNT_BT = 1 */
	odm_set_bb_reg(dm, 0x804, BIT(4),
		       1); /* Disable hw antsw & fast_train.antsw when GNT_BT=1 */

	/* Timming issue */
	odm_set_bb_reg(dm, 0x818, BIT(23) | BIT(22) | BIT(21) | BIT(20),
		       0); /*keep antidx after tx for ACK ( unit x 3.2 mu sec)*/
	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* response TX ant by RX ant */
	odm_set_mac_reg(dm, 0x668, BIT(3), 1);

}

void phydm_s0s1_sw_ant_div_init_8821c(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table = &dm->dm_swat_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8821C AntDiv_Init => ant_div_type=[ S0S1_SW_AntDiv]\n");

	/* Output Pin Settings */
	odm_set_mac_reg(dm, 0x4C, BIT(25), 0);

	odm_set_mac_reg(dm, 0x64, BIT(29), 1); /* PAPE by WLAN control */
	odm_set_mac_reg(dm, 0x64, BIT(28), 1); /* LNAON by WLAN control */

	odm_set_bb_reg(dm, 0xCB8, BIT(16), 0);

	odm_set_mac_reg(dm, 0x4C, BIT(23),
			0); /* select DPDT_P and DPDT_N as output pin */
	odm_set_mac_reg(dm, 0x4C, BIT(24), 1); /* by WLAN control */
	odm_set_bb_reg(dm, 0xCB4, 0xF, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, 0xF0, 8); /* DPDT_N = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB4, BIT(29), 0); /* DPDT_P non-inverse */
	odm_set_bb_reg(dm, 0xCB4, BIT(28), 1); /* DPDT_N inverse */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x00); /* bias */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	odm_set_bb_reg(dm, 0x800, BIT(25),
		       0); /* ANTSEL_CCK sent to the smart_antenna circuit */
	odm_set_bb_reg(dm, 0xA00, BIT(15),
		       0); /* CCK AntDiv function block enable */

	/* BT Coexistence */
	odm_set_bb_reg(dm, 0xCAC, BIT(9), 1); /* keep antsel_map when GNT_BT = 1 */
	odm_set_bb_reg(dm, 0x804, BIT(4),
		       1); /* Disable hw antsw & fast_train.antsw when GNT_BT=1 */

	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* response TX ant by RX ant */
	odm_set_mac_reg(dm, 0x668, BIT(3), 1);


	odm_set_bb_reg(dm, 0x900, BIT(18), 0);

	dm_swat_table->try_flag = SWAW_STEP_INIT;
	dm_swat_table->double_chk_flag = 0;
	dm_swat_table->cur_antenna = MAIN_ANT;
	dm_swat_table->pre_antenna = MAIN_ANT;
	dm_swat_table->swas_no_link_state = 0;

}
#endif /* #if (RTL8821C_SUPPORT == 1) */


#if (RTL8881A_SUPPORT == 1)
void odm_trx_hw_ant_div_init_8881a(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8881A AntDiv_Init => ant_div_type=[ CG_TRX_HW_ANTDIV (SPDT)]\n");

	/* Output Pin Settings */
	/* [SPDT related] */
	odm_set_mac_reg(dm, 0x4C, BIT(25), 0);
	odm_set_mac_reg(dm, 0x4C, BIT(26), 0);
	odm_set_bb_reg(dm, 0xCB4, BIT(31), 0); /* delay buffer */
	odm_set_bb_reg(dm, 0xCB4, BIT(22), 0);
	odm_set_bb_reg(dm, 0xCB4, BIT(24), 1);
	odm_set_bb_reg(dm, 0xCB0, 0xF00, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB0, 0xF0000, 8); /* DPDT_N = ANTSEL[0] */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x0); /* bias */
	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	/* 2 [--For HW Bug setting] */

	odm_set_bb_reg(dm, 0x900, BIT(18), 0); /* TX ant  by Reg */ /* A-cut bug */
}

#endif /* #if (RTL8881A_SUPPORT == 1) */


#if (RTL8812A_SUPPORT == 1)
void odm_trx_hw_ant_div_init_8812a(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8812A AntDiv_Init => ant_div_type=[ CG_TRX_HW_ANTDIV (SPDT)]\n");

	/* 3 */ /* 3 --RFE pin setting--------- */
	/* [BB] */
	odm_set_bb_reg(dm, 0x900, BIT(10) | BIT(9) | BIT(8),
		       0x0);	 /* disable SW switch */
	odm_set_bb_reg(dm, 0x900, BIT(17) | BIT(16), 0x0);
	odm_set_bb_reg(dm, 0x974, BIT(7) | BIT(6), 0x3);   /* in/out */
	odm_set_bb_reg(dm, 0xCB4, BIT(31), 0); /* delay buffer */
	odm_set_bb_reg(dm, 0xCB4, BIT(26), 0);
	odm_set_bb_reg(dm, 0xCB4, BIT(27), 1);
	odm_set_bb_reg(dm, 0xCB0, 0xF000000, 8); /* DPDT_P = ANTSEL[0] */
	odm_set_bb_reg(dm, 0xCB0, 0xF0000000, 8); /* DPDT_N = ANTSEL[0] */
	/* 3 ------------------------- */

	/* Mapping Table */
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE0, 0);
	odm_set_bb_reg(dm, 0xCA4, MASKBYTE1, 1);

	/* OFDM HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF, 0xA0); /* thershold */
	odm_set_bb_reg(dm, 0x8D4, 0x7FF000, 0x0); /* bias */
	odm_set_bb_reg(dm, 0x8CC, BIT(20) | BIT(19) | BIT(18),
		       3); /* settling time of antdiv by RF LNA = 100ns */

	/* CCK HW AntDiv Parameters */
	odm_set_bb_reg(dm, 0xA74, BIT(7), 1); /* patch for clk from 88M to 80M */
	odm_set_bb_reg(dm, 0xA0C, BIT(4), 1); /* do 64 samples */

	/* 2 [--For HW Bug setting] */

	odm_set_bb_reg(dm, 0x900, BIT(18), 0); /* TX ant  by Reg */ /* A-cut bug */

}

#endif /* #if (RTL8812A_SUPPORT == 1) */

#if (RTL8188F_SUPPORT == 1)
void odm_s0s1_sw_ant_div_init_8188f(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "***8188F AntDiv_Init => ant_div_type=[ S0S1_SW_AntDiv]\n");


	/*GPIO setting*/
	/*odm_set_mac_reg(dm, 0x64, BIT(18), 0); */
	/*odm_set_mac_reg(dm, 0x44, BIT(28)|BIT(27), 0);*/
	/*odm_set_mac_reg(dm, 0x44, BIT(20) | BIT(19), 0x3);*/	/*enable_output for P_GPIO[4:3]*/
	/*odm_set_mac_reg(dm, 0x44, BIT(12)|BIT(11), 0);*/ /*output value*/
	/*odm_set_mac_reg(dm, 0x40, BIT(1)|BIT(0), 0);*/		/*GPIO function*/

	if (dm->support_ic_type == ODM_RTL8188F) {
		if (dm->support_interface == ODM_ITRF_USB)
			odm_set_mac_reg(dm, 0x44, BIT(20) | BIT(19),
					0x3);	/*enable_output for P_GPIO[4:3]*/
		else if (dm->support_interface == ODM_ITRF_SDIO)
			odm_set_mac_reg(dm, 0x44, BIT(18), 0x1);	/*enable_output for P_GPIO[2]*/
	}

	fat_tab->is_become_linked  = false;
	dm_swat_table->try_flag = SWAW_STEP_INIT;
	dm_swat_table->double_chk_flag = 0;
}

void phydm_update_rx_idle_antenna_8188F(
	void	*dm_void,
	u32	default_ant
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	u8		codeword;

	if (dm->support_ic_type == ODM_RTL8188F) {
		if (dm->support_interface == ODM_ITRF_USB) {
			if (default_ant == ANT1_2G)
				codeword = 1; /*2'b01*/
			else
				codeword = 2;/*2'b10*/
			odm_set_mac_reg(dm, 0x44, (BIT(12) | BIT(11)),
					codeword); /*GPIO[4:3] output value*/
		} else if (dm->support_interface == ODM_ITRF_SDIO) {
			if (default_ant == ANT1_2G) {
				codeword = 0; /*1'b0*/
				odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8), 0x3);
				odm_set_bb_reg(dm, 0x860, BIT(9) | BIT(8), 0x1);
			} else {
				codeword = 1;/*1'b1*/
				odm_set_bb_reg(dm, 0x870, BIT(9) | BIT(8), 0x3);
				odm_set_bb_reg(dm, 0x860, BIT(9) | BIT(8), 0x2);
			}
			odm_set_mac_reg(dm, 0x44, BIT(10), codeword); /*GPIO[2] output value*/
		}
	}
}
#endif



#ifdef ODM_EVM_ENHANCE_ANTDIV
void phydm_evm_sw_antdiv_init(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	/*EVM enhance AntDiv method init----------------------------------------------------------------------*/
	fat_tab->evm_method_enable = 0;
	fat_tab->fat_state = NORMAL_STATE_MIAN;
	fat_tab->fat_state_cnt = 0;
	fat_tab->pre_antdiv_rssi = 0;

	dm->antdiv_intvl = 30;
	dm->antdiv_train_num = 2;
	odm_set_bb_reg(dm, 0x910, 0x3f, 0xf);
	dm->antdiv_evm_en = 1;
	/*dm->antdiv_period=1;*/
	dm->evm_antdiv_period = 3;
	dm->stop_antdiv_rssi_th = 3;
	dm->stop_antdiv_tp_th = 80;
	dm->antdiv_tp_period = 3;
	dm->stop_antdiv_tp_diff_th = 5;
}

void odm_evm_fast_ant_reset(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	fat_tab->evm_method_enable = 0;
	odm_ant_div_on_off(dm, ANTDIV_ON);
	fat_tab->fat_state = NORMAL_STATE_MIAN;
	fat_tab->fat_state_cnt = 0;
	dm->antdiv_period = 0;
	odm_set_mac_reg(dm, 0x608, BIT(8), 0);
}


void odm_evm_enhance_ant_div(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	main_rssi, aux_rssi ;
	u32	main_crc_utility = 0, aux_crc_utility = 0, utility_ratio = 1;
	u32	main_evm, aux_evm, diff_rssi = 0, diff_EVM = 0;
	u32	main_2ss_evm[2], aux_2ss_evm[2];
	u32	main_1ss_evm, aux_1ss_evm;
	u32	main_2ss_evm_sum, aux_2ss_evm_sum;
	u8	score_EVM = 0, score_CRC = 0;
	u8	rssi_larger_ant = 0;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;
	u32	value32, i;
	boolean main_above1 = false, aux_above1 = false;
	boolean force_antenna = false;
	struct cmn_sta_info	*sta;
	u32	antdiv_tp_main_avg, antdiv_tp_aux_avg;
	u8	curr_rssi, rssi_diff;
	u32	tp_diff;
	u8	tp_diff_return = 0, tp_return = 0, rssi_return = 0;
	u8	target_ant_evm_1ss, target_ant_evm_2ss;
	u8	decision_evm_ss;
	u8	next_ant;

	fat_tab->target_ant_enhance = 0xFF;

	if ((dm->support_ic_type & ODM_EVM_ENHANCE_ANTDIV_SUPPORT_IC)) {
		if (dm->is_one_entry_only) {
			/* PHYDM_DBG(dm,DBG_ANT_DIV, "[One Client only]\n"); */
			i = dm->one_entry_macid;
			sta = dm->phydm_sta_info[i];

			main_rssi = (fat_tab->main_ant_cnt[i] != 0) ? (fat_tab->main_ant_sum[i] /
					fat_tab->main_ant_cnt[i]) : 0;
			aux_rssi = (fat_tab->aux_ant_cnt[i] != 0) ? (fat_tab->aux_ant_sum[i] /
					fat_tab->aux_ant_cnt[i]) : 0;

			if ((main_rssi == 0 && aux_rssi != 0 && aux_rssi >= FORCE_RSSI_DIFF)
			    || (main_rssi != 0 && aux_rssi == 0 && main_rssi >= FORCE_RSSI_DIFF))
				diff_rssi = FORCE_RSSI_DIFF;
			else if (main_rssi != 0 && aux_rssi != 0)
				diff_rssi = (main_rssi >= aux_rssi) ? (main_rssi - aux_rssi) :
					    (aux_rssi - main_rssi);

			if (main_rssi >= aux_rssi)
				rssi_larger_ant = MAIN_ANT;
			else
				rssi_larger_ant = AUX_ANT;

			PHYDM_DBG(dm, DBG_ANT_DIV, "Main_Cnt=(( %d )), main_rssi=(( %d ))\n",
				  fat_tab->main_ant_cnt[i], main_rssi);
			PHYDM_DBG(dm, DBG_ANT_DIV, "Aux_Cnt=(( %d )), aux_rssi=(( %d ))\n",
				  fat_tab->aux_ant_cnt[i], aux_rssi);

			if (((main_rssi >= evm_rssi_th_high || aux_rssi >= evm_rssi_th_high)
			     || (fat_tab->evm_method_enable == 1))
			    /* && (diff_rssi <= FORCE_RSSI_DIFF + 1) */
			   ) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "> TH_H || evm_method_enable==1\n");

				if (((main_rssi >= evm_rssi_th_low) || (aux_rssi >= evm_rssi_th_low))) {
					PHYDM_DBG(dm, DBG_ANT_DIV, "> TH_L, fat_state_cnt =((%d))\n",
						  fat_tab->fat_state_cnt);

					/*Traning state: 0(alt) 1(ori) 2(alt) 3(ori)============================================================*/
					if (fat_tab->fat_state_cnt < ((dm->antdiv_train_num) << 1)) {
						if (fat_tab->fat_state_cnt == 0) {
							/*Reset EVM 1SS Method */
							fat_tab->main_ant_evm_sum[i] = 0;
							fat_tab->aux_ant_evm_sum[i] = 0;
							fat_tab->main_ant_evm_cnt[i] = 0;
							fat_tab->aux_ant_evm_cnt[i] = 0;
							/*Reset EVM 2SS Method */
							fat_tab->main_ant_evm_2ss_sum[i][0] = 0;
							fat_tab->main_ant_evm_2ss_sum[i][1] = 0;
							fat_tab->aux_ant_evm_2ss_sum[i][0] = 0;
							fat_tab->aux_ant_evm_2ss_sum[i][1] = 0;
							fat_tab->main_ant_evm_2ss_cnt[i] = 0;
							fat_tab->aux_ant_evm_2ss_cnt[i] = 0;
#if 0
							/*Reset TP Method */
							fat_tab->antdiv_tp_main = 0;
							fat_tab->antdiv_tp_aux = 0;
							fat_tab->antdiv_tp_main_cnt = 0;
							fat_tab->antdiv_tp_aux_cnt = 0;
#endif
							/*Reset CRC Method */
							fat_tab->main_crc32_ok_cnt = 0;
							fat_tab->main_crc32_fail_cnt = 0;
							fat_tab->aux_crc32_ok_cnt = 0;
							fat_tab->aux_crc32_fail_cnt = 0;

#ifdef SKIP_EVM_ANTDIV_TRAINING_PATCH
							if ((*dm->band_width == CHANNEL_WIDTH_20) && (sta->mimo_type == RF_2T2R)) {
								/*1. Skip training: RSSI*/
								/*PHYDM_DBG(pDM_Odm,DBG_ANT_DIV, "TargetAnt_enhance=((%d)), RxIdleAnt=((%d))\n", pDM_FatTable->TargetAnt_enhance, pDM_FatTable->RxIdleAnt);*/
								curr_rssi = (u8)((fat_tab->rx_idle_ant == MAIN_ANT) ? main_rssi :
										 aux_rssi);
								rssi_diff = (curr_rssi > fat_tab->pre_antdiv_rssi) ?
									    (curr_rssi - fat_tab->pre_antdiv_rssi) : (fat_tab->pre_antdiv_rssi -
											    curr_rssi);

								PHYDM_DBG(dm, DBG_ANT_DIV,
									  "[1] rssi_return, curr_rssi=((%d)), pre_rssi=((%d))\n", curr_rssi,
									  fat_tab->pre_antdiv_rssi);

								fat_tab->pre_antdiv_rssi = curr_rssi;
								if ((rssi_diff < (dm->stop_antdiv_rssi_th)) && (curr_rssi != 0))
									rssi_return = 1;

								/*2. Skip training: TP Diff*/
								tp_diff = (dm->rx_tp > fat_tab->pre_antdiv_tp) ? (dm->rx_tp  -
										fat_tab->pre_antdiv_tp) : (fat_tab->pre_antdiv_tp - dm->rx_tp);

								PHYDM_DBG(dm, DBG_ANT_DIV,
									  "[2] tp_diff_return, curr_tp=((%d)), pre_tp=((%d))\n", dm->rx_tp,
									  fat_tab->pre_antdiv_tp);
								fat_tab->pre_antdiv_tp = dm->rx_tp;
								if ((tp_diff < (u32)(dm->stop_antdiv_tp_diff_th)  && (dm->rx_tp != 0)))
									tp_diff_return = 1;

								PHYDM_DBG(dm, DBG_ANT_DIV, "[3] tp_return, curr_rx_tp=((%d))\n",
									  dm->rx_tp);
								/*3. Skip training: TP*/
								if (dm->rx_tp >= (u32)(dm->stop_antdiv_tp_th))
									tp_return = 1;

								PHYDM_DBG(dm, DBG_ANT_DIV,
									  "[4] Return {rssi, tp_diff, tp} = {%d, %d, %d}\n", rssi_return,
									  tp_diff_return, tp_return);
								/*4. Joint Return Decision*/
								if (tp_return) {
									if (tp_diff_return || rssi_diff) {
										PHYDM_DBG(dm, DBG_ANT_DIV, "***Return EVM SW AntDiv\n");
										return;
									}
								}
							}
#endif

							fat_tab->evm_method_enable = 1;
							odm_ant_div_on_off(dm, ANTDIV_OFF);
							dm->antdiv_period = dm->evm_antdiv_period;
							odm_set_mac_reg(dm, 0x608, BIT(8), 1); /*RCR accepts CRC32-Error packets*/

						}


						fat_tab->fat_state_cnt++;
						next_ant = (fat_tab->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
						odm_update_rx_idle_ant(dm, next_ant);
						odm_set_timer(dm, &dm->evm_fast_ant_training_timer, dm->antdiv_intvl); //ms

					}
					/*Decision state: 4==============================================================*/
					else {
						fat_tab->fat_state_cnt = 0;
						PHYDM_DBG(dm, DBG_ANT_DIV, "[Decisoin state ]\n");

						/* 3 [CRC32 statistic] */
#if 0
						if ((fat_tab->main_crc32_ok_cnt > ((fat_tab->aux_crc32_ok_cnt) << 1))
						    || ((diff_rssi >= 40) && (rssi_larger_ant == MAIN_ANT))) {
							fat_tab->target_ant_crc32 = MAIN_ANT;
							force_antenna = true;
							PHYDM_DBG(dm, DBG_ANT_DIV, "CRC32 Force Main\n");
						} else if ((fat_tab->aux_crc32_ok_cnt > ((fat_tab->main_crc32_ok_cnt) <<
								1)) || ((diff_rssi >= 40) && (rssi_larger_ant == AUX_ANT))) {
							fat_tab->target_ant_crc32 = AUX_ANT;
							force_antenna = true;
							PHYDM_DBG(dm, DBG_ANT_DIV, "CRC32 Force Aux\n");
						} else
#endif
						{
							if (fat_tab->main_crc32_fail_cnt <= 5)
								fat_tab->main_crc32_fail_cnt = 5;

							if (fat_tab->aux_crc32_fail_cnt <= 5)
								fat_tab->aux_crc32_fail_cnt = 5;

							if (fat_tab->main_crc32_ok_cnt > fat_tab->main_crc32_fail_cnt)
								main_above1 = true;

							if (fat_tab->aux_crc32_ok_cnt > fat_tab->aux_crc32_fail_cnt)
								aux_above1 = true;

							if (main_above1 == true && aux_above1 == false) {
								force_antenna = true;
								fat_tab->target_ant_crc32 = MAIN_ANT;
							} else if (main_above1 == false && aux_above1 == true) {
								force_antenna = true;
								fat_tab->target_ant_crc32 = AUX_ANT;
							} else if (main_above1 == true && aux_above1 == true) {
								main_crc_utility = ((fat_tab->main_crc32_ok_cnt) << 7) /
										   fat_tab->main_crc32_fail_cnt;
								aux_crc_utility = ((fat_tab->aux_crc32_ok_cnt) << 7) /
										  fat_tab->aux_crc32_fail_cnt;
								fat_tab->target_ant_crc32 = (main_crc_utility == aux_crc_utility) ?
											    (fat_tab->pre_target_ant_enhance) : ((main_crc_utility >= aux_crc_utility)
													    ? MAIN_ANT : AUX_ANT);

								if (main_crc_utility != 0 && aux_crc_utility != 0) {
									if (main_crc_utility >= aux_crc_utility)
										utility_ratio = (main_crc_utility << 1) / aux_crc_utility;
									else
										utility_ratio = (aux_crc_utility << 1) / main_crc_utility;
								}
							} else if (main_above1 == false && aux_above1 == false) {
								if (fat_tab->main_crc32_ok_cnt == 0)
									fat_tab->main_crc32_ok_cnt = 1;
								if (fat_tab->aux_crc32_ok_cnt == 0)
									fat_tab->aux_crc32_ok_cnt = 1;

								main_crc_utility = ((fat_tab->main_crc32_fail_cnt) << 7) /
										   fat_tab->main_crc32_ok_cnt;
								aux_crc_utility = ((fat_tab->aux_crc32_fail_cnt) << 7) /
										  fat_tab->aux_crc32_ok_cnt;
								fat_tab->target_ant_crc32 = (main_crc_utility == aux_crc_utility) ?
											    (fat_tab->pre_target_ant_enhance) : ((main_crc_utility <= aux_crc_utility)
													    ? MAIN_ANT : AUX_ANT);

								if (main_crc_utility != 0 && aux_crc_utility != 0) {
									if (main_crc_utility >= aux_crc_utility)
										utility_ratio = (main_crc_utility << 1) / (aux_crc_utility);
									else
										utility_ratio = (aux_crc_utility << 1) / (main_crc_utility);
								}
							}
						}
						odm_set_mac_reg(dm, 0x608, BIT(8), 0);/* NOT Accept CRC32 Error packets. */
						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "MAIN_CRC: Ok=((%d)), Fail = ((%d)), Utility = ((%d))\n",
							  fat_tab->main_crc32_ok_cnt, fat_tab->main_crc32_fail_cnt,
							  main_crc_utility);
						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "AUX__CRC: Ok=((%d)), Fail = ((%d)), Utility = ((%d))\n",
							  fat_tab->aux_crc32_ok_cnt, fat_tab->aux_crc32_fail_cnt, aux_crc_utility);
						PHYDM_DBG(dm, DBG_ANT_DIV, "***1.TargetAnt_CRC32 = ((%s))\n",
							  (fat_tab->target_ant_crc32 == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

						/* 3 [EVM statistic] */
						/*1SS EVM*/
						main_1ss_evm = (fat_tab->main_ant_evm_cnt[i] != 0) ?
							       (fat_tab->main_ant_evm_sum[i] / fat_tab->main_ant_evm_cnt[i]) : 0;
						aux_1ss_evm = (fat_tab->aux_ant_evm_cnt[i] != 0) ?
							      (fat_tab->aux_ant_evm_sum[i] / fat_tab->aux_ant_evm_cnt[i]) : 0;
						target_ant_evm_1ss = (main_1ss_evm == aux_1ss_evm) ?
								     (fat_tab->pre_target_ant_enhance) : ((main_1ss_evm >= aux_1ss_evm) ?
										     MAIN_ANT : AUX_ANT);

						PHYDM_DBG(dm, DBG_ANT_DIV, "Cnt = ((%d)), Main1ss_EVM= ((  %d ))\n",
							  fat_tab->main_ant_evm_cnt[i], main_1ss_evm);
						PHYDM_DBG(dm, DBG_ANT_DIV, "Cnt = ((%d)), Aux_1ss_EVM = ((  %d ))\n",
							  fat_tab->main_ant_evm_cnt[i], aux_1ss_evm);

						/*2SS EVM*/
						main_2ss_evm[0] = (fat_tab->main_ant_evm_2ss_cnt[i] != 0) ?
								  (fat_tab->main_ant_evm_2ss_sum[i][0] / fat_tab->main_ant_evm_2ss_cnt[i]) :
								  0;
						main_2ss_evm[1] = (fat_tab->main_ant_evm_2ss_cnt[i] != 0) ?
								  (fat_tab->main_ant_evm_2ss_sum[i][1] / fat_tab->main_ant_evm_2ss_cnt[i]) :
								  0;
						main_2ss_evm_sum = main_2ss_evm[0] + main_2ss_evm[1];

						aux_2ss_evm[0] = (fat_tab->aux_ant_evm_2ss_cnt[i] != 0) ?
								 (fat_tab->aux_ant_evm_2ss_sum[i][0] / fat_tab->aux_ant_evm_2ss_cnt[i]) : 0;
						aux_2ss_evm[1] = (fat_tab->aux_ant_evm_2ss_cnt[i] != 0) ?
								 (fat_tab->aux_ant_evm_2ss_sum[i][1] / fat_tab->aux_ant_evm_2ss_cnt[i]) : 0;
						aux_2ss_evm_sum = aux_2ss_evm[0] + aux_2ss_evm[1];

						target_ant_evm_2ss = (main_2ss_evm_sum == aux_2ss_evm_sum) ?
								     (fat_tab->pre_target_ant_enhance) : ((main_2ss_evm_sum >= aux_2ss_evm_sum)
										     ? MAIN_ANT : AUX_ANT);

						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "Cnt = ((%d)), Main2ss_EVM{A,B,Sum} = {%d, %d, %d}\n",
							  fat_tab->main_ant_evm_2ss_cnt[i], main_2ss_evm[0], main_2ss_evm[1],
							  main_2ss_evm_sum);
						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "Cnt = ((%d)), Aux_2ss_EVM{A,B,Sum} = {%d, %d, %d}\n",
							  fat_tab->aux_ant_evm_2ss_cnt[i], aux_2ss_evm[0], aux_2ss_evm[1],
							  aux_2ss_evm_sum);

						if ((main_2ss_evm_sum + aux_2ss_evm_sum) != 0) {
							decision_evm_ss = 2;
							main_evm = main_2ss_evm_sum;
							aux_evm = aux_2ss_evm_sum;
							fat_tab->target_ant_evm = target_ant_evm_2ss;
						} else {
							decision_evm_ss = 1;
							main_evm = main_1ss_evm;
							aux_evm = aux_1ss_evm;
							fat_tab->target_ant_evm = target_ant_evm_1ss;
						}

						if ((main_evm == 0 || aux_evm == 0))
							diff_EVM = 100;
						else if (main_evm >= aux_evm)
							diff_EVM = main_evm - aux_evm;
						else
							diff_EVM = aux_evm - main_evm;

						PHYDM_DBG(dm, DBG_ANT_DIV, "***2.TargetAnt_EVM((%d-ss)) = ((%s))\n",
							  decision_evm_ss, (fat_tab->target_ant_evm == MAIN_ANT) ? "MAIN_ANT" :
							  "AUX_ANT");


						//3 [TP statistic]
						antdiv_tp_main_avg = (fat_tab->antdiv_tp_main_cnt != 0) ?
								     (fat_tab->antdiv_tp_main / fat_tab->antdiv_tp_main_cnt) : 0;
						antdiv_tp_aux_avg = (fat_tab->antdiv_tp_aux_cnt != 0) ?
								    (fat_tab->antdiv_tp_aux / fat_tab->antdiv_tp_aux_cnt) : 0;
						fat_tab->target_ant_tp = (antdiv_tp_main_avg == antdiv_tp_aux_avg) ?
									 (fat_tab->pre_target_ant_enhance) : ((antdiv_tp_main_avg >=
											 antdiv_tp_aux_avg) ? MAIN_ANT : AUX_ANT);

						PHYDM_DBG(dm, DBG_ANT_DIV, "Cnt = ((%d)), Main_TP = ((%d))\n",
							  fat_tab->antdiv_tp_main_cnt, antdiv_tp_main_avg);
						PHYDM_DBG(dm, DBG_ANT_DIV, "Cnt = ((%d)), Aux_TP = ((%d))\n",
							  fat_tab->antdiv_tp_aux_cnt, antdiv_tp_aux_avg);
						PHYDM_DBG(dm, DBG_ANT_DIV, "***3.TargetAnt_TP = ((%s))\n",
							  (fat_tab->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

						/*Reset TP Method */
						fat_tab->antdiv_tp_main = 0;
						fat_tab->antdiv_tp_aux = 0;
						fat_tab->antdiv_tp_main_cnt = 0;
						fat_tab->antdiv_tp_aux_cnt = 0;

						/* 2 [ Decision state ] */
						if (fat_tab->target_ant_evm == fat_tab->target_ant_crc32) {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "Decision type 1, CRC_utility = ((%d)), EVM_diff = ((%d))\n",
								  utility_ratio, diff_EVM);

							if ((utility_ratio < 2 && force_antenna == false) && diff_EVM <= 30)
								fat_tab->target_ant_enhance = fat_tab->pre_target_ant_enhance;
							else
								fat_tab->target_ant_enhance = fat_tab->target_ant_evm;
						}
#if 0
						else if ((diff_EVM <= 50 && (utility_ratio > 4 && force_antenna == false))
							 || (force_antenna == true)) {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "Decision type 2, CRC_utility = ((%d)), EVM_diff = ((%d))\n",
								  utility_ratio, diff_EVM);
							fat_tab->target_ant_enhance = fat_tab->target_ant_crc32;
						}
#endif
						else if (diff_EVM >= 20) {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "Decision type 3, CRC_utility = ((%d)), EVM_diff = ((%d))\n",
								  utility_ratio, diff_EVM);
							fat_tab->target_ant_enhance = fat_tab->target_ant_evm;
						} else if (utility_ratio >= 6 && force_antenna == false) {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "Decision type 4, CRC_utility = ((%d)), EVM_diff = ((%d))\n",
								  utility_ratio, diff_EVM);
							fat_tab->target_ant_enhance = fat_tab->target_ant_crc32;
						} else {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "Decision type 5, CRC_utility = ((%d)), EVM_diff = ((%d))\n",
								  utility_ratio, diff_EVM);

							if (force_antenna == true)
								score_CRC = 2;
							else if (utility_ratio >= 5) /*>2.5*/
								score_CRC = 2;
							else if (utility_ratio >= 4) /*>2*/
								score_CRC = 1;
							else
								score_CRC = 0;

							if (diff_EVM >= 15)
								score_EVM = 3;
							else if (diff_EVM >= 10)
								score_EVM = 2;
							else if (diff_EVM >= 5)
								score_EVM = 1;
							else
								score_EVM = 0;

							if (score_CRC > score_EVM)
								fat_tab->target_ant_enhance = fat_tab->target_ant_crc32;
							else if (score_CRC < score_EVM)
								fat_tab->target_ant_enhance = fat_tab->target_ant_evm;
							else
								fat_tab->target_ant_enhance = fat_tab->pre_target_ant_enhance;
						}
						fat_tab->pre_target_ant_enhance = fat_tab->target_ant_enhance;

						PHYDM_DBG(dm, DBG_ANT_DIV, "*** 4.TargetAnt_enhance = (( %s ))******\n",
							  (fat_tab->target_ant_enhance == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");


					}
				} else { /* RSSI< = evm_rssi_th_low */
					PHYDM_DBG(dm, DBG_ANT_DIV, "[ <TH_L: escape from > TH_L ]\n");
					odm_evm_fast_ant_reset(dm);
				}
			} else {
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "[escape from> TH_H || evm_method_enable==1]\n");
				odm_evm_fast_ant_reset(dm);
			}
		} else {
			PHYDM_DBG(dm, DBG_ANT_DIV, "[multi-Client]\n");
			odm_evm_fast_ant_reset(dm);
		}
	}
}

void odm_evm_fast_ant_training_callback(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV, "******AntDiv_Callback******\n");
	odm_hw_ant_div(dm);
}
#endif

void odm_hw_ant_div(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	u32	i, min_max_rssi = 0xFF,  ant_div_max_rssi = 0, max_rssi = 0,
		   local_max_rssi;
	u32	main_rssi, aux_rssi, mian_cnt, aux_cnt;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;
	u8	rx_idle_ant = fat_tab->rx_idle_ant, target_ant = 7;
	struct phydm_dig_struct	*dig_t = &dm->dm_dig_table;
	struct cmn_sta_info	*sta;

#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	struct _BF_DIV_COEX_    *dm_bdc_table = &dm->dm_bdc_table;
	u32	TH1 = 500000;
	u32	TH2 = 10000000;
	u32	ma_rx_temp, degrade_TP_temp, improve_TP_temp;
	u8	monitor_rssi_threshold = 30;

	dm_bdc_table->BF_pass = true;
	dm_bdc_table->DIV_pass = true;
	dm_bdc_table->is_all_div_sta_idle = true;
	dm_bdc_table->is_all_bf_sta_idle = true;
	dm_bdc_table->num_bf_tar = 0 ;
	dm_bdc_table->num_div_tar = 0;
	dm_bdc_table->num_client = 0;
#endif
#endif

	if (!dm->is_linked) { /* is_linked==False */
		PHYDM_DBG(dm, DBG_ANT_DIV, "[No Link!!!]\n");

		if (fat_tab->is_become_linked == true) {
			odm_ant_div_on_off(dm, ANTDIV_OFF);
			odm_update_rx_idle_ant(dm, MAIN_ANT);
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
			dm->antdiv_period = 0;

			fat_tab->is_become_linked = dm->is_linked;
		}
		return;
	} else {
		if (fat_tab->is_become_linked == false) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "[Linked !!!]\n");
			odm_ant_div_on_off(dm, ANTDIV_ON);
			/*odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);*/

			/* if(dm->support_ic_type == ODM_RTL8821 ) */
			/* odm_set_bb_reg(dm, 0x800, BIT(25), 0); */ /* CCK AntDiv function disable */

			/* #if(DM_ODM_SUPPORT_TYPE  == ODM_AP) */
			/* else if(dm->support_ic_type == ODM_RTL8881A) */
			/* odm_set_bb_reg(dm, 0x800, BIT(25), 0); */ /* CCK AntDiv function disable */
			/* #endif */

			/* else if(dm->support_ic_type == ODM_RTL8723B ||dm->support_ic_type == ODM_RTL8812) */
			/* odm_set_bb_reg(dm, 0xA00, BIT(15), 0); */ /* CCK AntDiv function disable */

			fat_tab->is_become_linked = dm->is_linked;

			if (dm->support_ic_type == ODM_RTL8723B
			    && dm->ant_div_type == CG_TRX_HW_ANTDIV) {
				odm_set_bb_reg(dm, 0x930, 0xF0,
					       8); /* DPDT_P = ANTSEL[0]   */ /* for 8723B AntDiv function patch.  BB  Dino  130412 */
				odm_set_bb_reg(dm, 0x930, 0xF, 8); /* DPDT_N = ANTSEL[0] */
			}

			/* 2 BDC Init */
#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
			odm_bdc_init(dm);
#endif
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
			odm_evm_fast_ant_reset(dm);
#endif
		}
	}

	if (*fat_tab->p_force_tx_ant_by_desc == false) {
		if (dm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);
	}

#ifdef ODM_EVM_ENHANCE_ANTDIV
	if (dm->antdiv_evm_en == 1) {
		odm_evm_enhance_ant_div(dm);
		if (fat_tab->fat_state_cnt != 0)
			return;
	} else
		odm_evm_fast_ant_reset(dm);
#endif

	/* 2 BDC mode Arbitration */
#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	if (dm->antdiv_evm_en == 0 || fat_tab->evm_method_enable == 0)
		odm_bf_ant_div_mode_arbitration(dm);
#endif
#endif

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		sta = dm->phydm_sta_info[i];
		if (is_sta_active(sta)) {
			/* 2 Caculate RSSI per Antenna */
			if ((fat_tab->main_ant_cnt[i] != 0) || (fat_tab->aux_ant_cnt[i] != 0)) {
				mian_cnt = fat_tab->main_ant_cnt[i];
				aux_cnt = fat_tab->aux_ant_cnt[i];
				main_rssi = (mian_cnt != 0) ? (fat_tab->main_ant_sum[i] / mian_cnt) : 0;
				aux_rssi = (aux_cnt != 0) ? (fat_tab->aux_ant_sum[i] / aux_cnt) : 0;
				target_ant = (mian_cnt == aux_cnt) ? fat_tab->rx_idle_ant : ((
							mian_cnt >= aux_cnt) ? MAIN_ANT : AUX_ANT); /*Use counter number for OFDM*/

			} else {	/*CCK only case*/
				mian_cnt = fat_tab->main_ant_cnt_cck[i];
				aux_cnt = fat_tab->aux_ant_cnt_cck[i];
				main_rssi = (mian_cnt != 0) ? (fat_tab->main_ant_sum_cck[i] / mian_cnt) :
					    0;
				aux_rssi = (aux_cnt != 0) ? (fat_tab->aux_ant_sum_cck[i] / aux_cnt) : 0;
				target_ant = (main_rssi == aux_rssi) ? fat_tab->rx_idle_ant : ((
							main_rssi >= aux_rssi) ? MAIN_ANT :
						AUX_ANT); /*Use RSSI for CCK only case*/
			}

			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "*** Client[ %d ] : Main_Cnt = (( %d ))  ,  CCK_Main_Cnt = (( %d )) ,  main_rssi= ((  %d ))\n",
				  i, fat_tab->main_ant_cnt[i], fat_tab->main_ant_cnt_cck[i], main_rssi);
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "*** Client[ %d ] : Aux_Cnt   = (( %d ))  , CCK_Aux_Cnt   = (( %d )) ,  aux_rssi = ((  %d ))\n",
				  i, fat_tab->aux_ant_cnt[i], fat_tab->aux_ant_cnt_cck[i], aux_rssi);
			/* PHYDM_DBG(dm,DBG_ANT_DIV, "*** MAC ID:[ %d ] , target_ant = (( %s ))\n", i ,( target_ant ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"); */

			local_max_rssi = (main_rssi > aux_rssi) ? main_rssi : aux_rssi;
			/* 2 Select max_rssi for DIG */
			if ((local_max_rssi > ant_div_max_rssi) && (local_max_rssi < 40))
				ant_div_max_rssi = local_max_rssi;
			if (local_max_rssi > max_rssi)
				max_rssi = local_max_rssi;

			/* 2 Select RX Idle Antenna */
			if ((local_max_rssi != 0) && (local_max_rssi < min_max_rssi)) {
				rx_idle_ant = target_ant;
				min_max_rssi = local_max_rssi;
			}

#ifdef ODM_EVM_ENHANCE_ANTDIV
			if (dm->antdiv_evm_en == 1) {
				if (fat_tab->target_ant_enhance != 0xFF) {
					target_ant = fat_tab->target_ant_enhance;
					rx_idle_ant = fat_tab->target_ant_enhance;
				}
			}
#endif

			/* 2 Select TX Antenna */
			if (dm->ant_div_type != CGCS_RX_HW_ANTDIV) {
#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
				if (dm_bdc_table->w_bfee_client[i] == 0)
#endif
#endif
				{
					odm_update_tx_ant(dm, target_ant, i);
				}
			}

			/* ------------------------------------------------------------ */

#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

			dm_bdc_table->num_client++;

			if (dm_bdc_table->bdc_mode == BDC_MODE_2
			    || dm_bdc_table->bdc_mode == BDC_MODE_3) {
				/* 2 Byte counter */

				ma_rx_temp = sta->rx_moving_average_tp; /* RX  TP   ( bit /sec) */

				if (dm_bdc_table->BDC_state == bdc_bfer_train_state)
					dm_bdc_table->MA_rx_TP_DIV[i] =  ma_rx_temp ;
				else
					dm_bdc_table->MA_rx_TP[i] = ma_rx_temp ;

				if ((ma_rx_temp < TH2)   && (ma_rx_temp > TH1)
				    && (local_max_rssi <= monitor_rssi_threshold)) {
					if (dm_bdc_table->w_bfer_client[i] == 1) { /* Bfer_Target */
						dm_bdc_table->num_bf_tar++;

						if (dm_bdc_table->BDC_state == BDC_DECISION_STATE
						    && dm_bdc_table->bdc_try_flag == 0) {
							improve_TP_temp = (dm_bdc_table->MA_rx_TP_DIV[i] * 9) >> 3 ; /* * 1.125 */
							dm_bdc_table->BF_pass = (dm_bdc_table->MA_rx_TP[i] > improve_TP_temp) ?
										true : false;
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "*** Client[ %d ] :  { MA_rx_TP,improve_TP_temp, MA_rx_TP_DIV,  BF_pass}={ %d,  %d, %d , %d }\n",
								  i, dm_bdc_table->MA_rx_TP[i], improve_TP_temp,
								  dm_bdc_table->MA_rx_TP_DIV[i], dm_bdc_table->BF_pass);
						}
					} else { /* DIV_Target */
						dm_bdc_table->num_div_tar++;

						if (dm_bdc_table->BDC_state == BDC_DECISION_STATE
						    && dm_bdc_table->bdc_try_flag == 0) {
							degrade_TP_temp = (dm_bdc_table->MA_rx_TP_DIV[i] * 5) >> 3; /* * 0.625 */
							dm_bdc_table->DIV_pass = (dm_bdc_table->MA_rx_TP[i] > degrade_TP_temp) ?
										 true : false;
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "*** Client[ %d ] :  { MA_rx_TP, degrade_TP_temp, MA_rx_TP_DIV,  DIV_pass}=\n{ %d,  %d, %d , %d }\n",
								  i, dm_bdc_table->MA_rx_TP[i], degrade_TP_temp,
								  dm_bdc_table->MA_rx_TP_DIV[i], dm_bdc_table->DIV_pass);
						}
					}
				}

				if (ma_rx_temp > TH1) {
					if (dm_bdc_table->w_bfer_client[i] == 1) /* Bfer_Target */
						dm_bdc_table->is_all_bf_sta_idle = false;
					else/* DIV_Target */
						dm_bdc_table->is_all_div_sta_idle = false;
				}

				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "*** Client[ %d ] :  { BFmeeCap, BFmerCap}  = { %d , %d }\n", i,
					  dm_bdc_table->w_bfee_client[i], dm_bdc_table->w_bfer_client[i]);

				if (dm_bdc_table->BDC_state == bdc_bfer_train_state)
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "*** Client[ %d ] :    MA_rx_TP_DIV = (( %d ))\n", i,
						  dm_bdc_table->MA_rx_TP_DIV[i]);

				else
					PHYDM_DBG(dm, DBG_ANT_DIV, "*** Client[ %d ] :    MA_rx_TP = (( %d ))\n",
						  i, dm_bdc_table->MA_rx_TP[i]);

			}
#endif
#endif

		}

#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
		if (dm_bdc_table->bdc_try_flag == 0)
#endif
#endif
		{
			phydm_antdiv_reset_statistic(dm, i);
		}
	}



	/* 2 Set RX Idle Antenna & TX Antenna(Because of HW Bug ) */
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	PHYDM_DBG(dm, DBG_ANT_DIV, "*** rx_idle_ant = (( %s ))\n",
		  (rx_idle_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	if (dm_bdc_table->bdc_mode == BDC_MODE_1
	    || dm_bdc_table->bdc_mode == BDC_MODE_3) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "*** bdc_rx_idle_update_counter = (( %d ))\n",
			  dm_bdc_table->bdc_rx_idle_update_counter);

		if (dm_bdc_table->bdc_rx_idle_update_counter == 1) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "***Update RxIdle Antenna!!!\n");
			dm_bdc_table->bdc_rx_idle_update_counter = 30;
			odm_update_rx_idle_ant(dm, rx_idle_ant);
		} else {
			dm_bdc_table->bdc_rx_idle_update_counter--;
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "***NOT update RxIdle Antenna because of BF  ( need to fix TX-ant)\n");
		}
	} else
#endif
#endif
		odm_update_rx_idle_ant(dm, rx_idle_ant);
#else

	odm_update_rx_idle_ant(dm, rx_idle_ant);

#endif/* #if(DM_ODM_SUPPORT_TYPE  == ODM_AP) */



	/* 2 BDC Main Algorithm */
#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	if (dm->antdiv_evm_en == 0 || fat_tab->evm_method_enable == 0)
		odm_bd_ccoex_bfee_rx_div_arbitration(dm);

	dm_bdc_table->num_txbfee_client = 0;
	dm_bdc_table->num_txbfer_client = 0;
#endif
#endif

	if (ant_div_max_rssi == 0)
		dig_t->ant_div_rssi_max = dm->rssi_min;
	else
		dig_t->ant_div_rssi_max = ant_div_max_rssi;

	PHYDM_DBG(dm, DBG_ANT_DIV, "***AntDiv End***\n\n");
}



#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY

void odm_s0s1_sw_ant_div_reset(
	void		*dm_void
)
{
	struct dm_struct	*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch		*dm_swat_table	= &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab		= &dm->dm_fat_table;

	fat_tab->is_become_linked  = false;
	dm_swat_table->try_flag = SWAW_STEP_INIT;
	dm_swat_table->double_chk_flag = 0;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "odm_s0s1_sw_ant_div_reset(): fat_tab->is_become_linked = %d\n",
		  fat_tab->is_become_linked);
}

void odm_s0s1_sw_ant_div(
	void			*dm_void,
	u8			step
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch			*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	u32			i, min_max_rssi = 0xFF, local_max_rssi, local_min_rssi;
	u32			main_rssi, aux_rssi;
	u8			high_traffic_train_time_u = 0x32, high_traffic_train_time_l = 0,
				train_time_temp;
	u8			low_traffic_train_time_u = 200, low_traffic_train_time_l = 0;
	u8			rx_idle_ant = dm_swat_table->pre_antenna, target_ant, next_ant = 0;
	struct cmn_sta_info		*entry = NULL;
	u32			value32;
	u32			main_ant_sum = 0;
	u32			aux_ant_sum = 0;
	u32			main_ant_cnt = 0;
	u32			aux_ant_cnt = 0;


	if (!dm->is_linked) { /* is_linked==False */
		PHYDM_DBG(dm, DBG_ANT_DIV, "[No Link!!!]\n");
		if (fat_tab->is_become_linked == true) {
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
			if (dm->support_ic_type == ODM_RTL8723B) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "Set REG 948[9:6]=0x0\n");
				odm_set_bb_reg(dm, 0x948, (BIT(9) | BIT(8) | BIT(7) | BIT(6)), 0x0);
			}
			fat_tab->is_become_linked = dm->is_linked;
		}
		return;
	} else {
		if (fat_tab->is_become_linked == false) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "[Linked !!!]\n");

			if (dm->support_ic_type == ODM_RTL8723B) {
				value32 = odm_get_bb_reg(dm, 0x864, BIT(5) | BIT(4) | BIT(3));

#if (RTL8723B_SUPPORT == 1)
				if (value32 == 0x0)
					odm_update_rx_idle_ant_8723b(dm, MAIN_ANT, ANT1_2G, ANT2_2G);
				else if (value32 == 0x1)
					odm_update_rx_idle_ant_8723b(dm, AUX_ANT, ANT2_2G, ANT1_2G);
#endif

				PHYDM_DBG(dm, DBG_ANT_DIV, "8723B: First link! Force antenna to  %s\n",
					  (value32 == 0x0 ? "MAIN" : "AUX"));
			}
			fat_tab->is_become_linked = dm->is_linked;
		}
	}

	if (*fat_tab->p_force_tx_ant_by_desc == false) {
		if (dm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);
	}

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[%d] { try_flag=(( %d )), step=(( %d )), double_chk_flag = (( %d )) }\n",
		  __LINE__, dm_swat_table->try_flag, step, dm_swat_table->double_chk_flag);

	/* Handling step mismatch condition. */
	/* Peak step is not finished at last time. Recover the variable and check again. */
	if (step != dm_swat_table->try_flag) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[step != try_flag]    Need to Reset After Link\n");
		odm_sw_ant_div_rest_after_link(dm);
	}

	if (dm_swat_table->try_flag == SWAW_STEP_INIT) {
		dm_swat_table->try_flag = SWAW_STEP_PEEK;
		dm_swat_table->train_time_flag = 0;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[set try_flag = 0]  Prepare for peek!\n\n");
		return;

	} else {
		/* 1 Normal state (Begin Trying) */
		if (dm_swat_table->try_flag == SWAW_STEP_PEEK) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "TxOkCnt=(( %llu )), RxOkCnt=(( %llu )), traffic_load = (%d))\n",
				  dm->cur_tx_ok_cnt, dm->cur_rx_ok_cnt, dm->traffic_load);

			if (dm->traffic_load == TRAFFIC_HIGH) {
				train_time_temp = dm_swat_table->train_time ;

				if (dm_swat_table->train_time_flag == 3) {
					high_traffic_train_time_l = 0xa;

					if (train_time_temp <= 16)
						train_time_temp = high_traffic_train_time_l;
					else
						train_time_temp -= 16;

				} else if (dm_swat_table->train_time_flag == 2) {
					train_time_temp -= 8;
					high_traffic_train_time_l = 0xf;
				} else if (dm_swat_table->train_time_flag == 1) {
					train_time_temp -= 4;
					high_traffic_train_time_l = 0x1e;
				} else if (dm_swat_table->train_time_flag == 0) {
					train_time_temp += 8;
					high_traffic_train_time_l = 0x28;
				}

				if (dm->support_ic_type == ODM_RTL8188F) {
					if (dm->support_interface == ODM_ITRF_SDIO)
						high_traffic_train_time_l += 0xa;
				}

				/* PHYDM_DBG(dm,DBG_ANT_DIV, "*** train_time_temp = ((%d))\n",train_time_temp); */

				/* -- */
				if (train_time_temp > high_traffic_train_time_u)
					train_time_temp = high_traffic_train_time_u;

				else if (train_time_temp < high_traffic_train_time_l)
					train_time_temp = high_traffic_train_time_l;

				dm_swat_table->train_time = train_time_temp; /*10ms~200ms*/

				PHYDM_DBG(dm, DBG_ANT_DIV, "train_time_flag=((%d)), train_time=((%d))\n",
					  dm_swat_table->train_time_flag, dm_swat_table->train_time);

			} else if ((dm->traffic_load == TRAFFIC_MID)
				   || (dm->traffic_load == TRAFFIC_LOW)) {
				train_time_temp = dm_swat_table->train_time ;

				if (dm_swat_table->train_time_flag == 3) {
					low_traffic_train_time_l = 10;
					if (train_time_temp < 50)
						train_time_temp = low_traffic_train_time_l;
					else
						train_time_temp -= 50;
				} else if (dm_swat_table->train_time_flag == 2) {
					train_time_temp -= 30;
					low_traffic_train_time_l = 36;
				} else if (dm_swat_table->train_time_flag == 1) {
					train_time_temp -= 10;
					low_traffic_train_time_l = 40;
				} else {
					train_time_temp += 10;
					low_traffic_train_time_l = 50;
				}

				if (dm->support_ic_type == ODM_RTL8188F) {
					if (dm->support_interface == ODM_ITRF_SDIO)
						low_traffic_train_time_l += 10;
				}

				/* -- */
				if (train_time_temp >= low_traffic_train_time_u)
					train_time_temp = low_traffic_train_time_u;

				else if (train_time_temp <= low_traffic_train_time_l)
					train_time_temp = low_traffic_train_time_l;

				dm_swat_table->train_time = train_time_temp; /*10ms~200ms*/

				PHYDM_DBG(dm, DBG_ANT_DIV, "train_time_flag=((%d)) , train_time=((%d))\n",
					  dm_swat_table->train_time_flag, dm_swat_table->train_time);

			} else {
				dm_swat_table->train_time = 0xc8; /*200ms*/

			}

			/* ----------------- */

			PHYDM_DBG(dm, DBG_ANT_DIV, "Current min_max_rssi is ((%d))\n",
				  fat_tab->min_max_rssi);

			/* ---reset index--- */
			if (dm_swat_table->reset_idx >= RSSI_CHECK_RESET_PERIOD) {
				fat_tab->min_max_rssi = 0;
				dm_swat_table->reset_idx = 0;
			}
			PHYDM_DBG(dm, DBG_ANT_DIV, "reset_idx = (( %d ))\n",
				  dm_swat_table->reset_idx);

			dm_swat_table->reset_idx++;

			/* ---double check flag--- */
			if ((fat_tab->min_max_rssi > RSSI_CHECK_THRESHOLD)
			    && (dm_swat_table->double_chk_flag == 0)) {
				PHYDM_DBG(dm, DBG_ANT_DIV, " min_max_rssi is ((%d)), and > %d\n",
					  fat_tab->min_max_rssi, RSSI_CHECK_THRESHOLD);

				dm_swat_table->double_chk_flag = 1;
				dm_swat_table->try_flag = SWAW_STEP_DETERMINE;
				dm_swat_table->rssi_trying = 0;

				PHYDM_DBG(dm, DBG_ANT_DIV, "Test the current ant for (( %d )) ms again\n",
					  dm_swat_table->train_time);
				odm_update_rx_idle_ant(dm, fat_tab->rx_idle_ant);
				odm_set_timer(dm,
					      &dm_swat_table->phydm_sw_antenna_switch_timer,
					      dm_swat_table->train_time); /*ms*/
				return;
			}

			next_ant = (fat_tab->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;

			dm_swat_table->try_flag = SWAW_STEP_DETERMINE;

			if (dm_swat_table->reset_idx <= 1)
				dm_swat_table->rssi_trying = 2;
			else
				dm_swat_table->rssi_trying = 1;

			odm_s0s1_sw_ant_div_by_ctrl_frame(dm, SWAW_STEP_PEEK);
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[set try_flag=1]  Normal state:  Begin Trying!!\n");

		} else if ((dm_swat_table->try_flag == SWAW_STEP_DETERMINE)
			   && (dm_swat_table->double_chk_flag == 0)) {
			next_ant = (fat_tab->rx_idle_ant  == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
			dm_swat_table->rssi_trying--;
		}

		/* 1 Decision state */
		if ((dm_swat_table->try_flag == SWAW_STEP_DETERMINE)
		    && (dm_swat_table->rssi_trying == 0)) {
			boolean is_by_ctrl_frame = false;
			u64	pkt_cnt_total = 0;

			for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
				entry = dm->phydm_sta_info[i];
				if (is_sta_active(entry)) {
					/* 2 Caculate RSSI per Antenna */
#if 0
					main_ant_sum = (u32)fat_tab->main_ant_sum[i] + (u32)
						       fat_tab->main_ant_sum_cck[i];
					aux_ant_sum = (u32)fat_tab->aux_ant_sum[i] + (u32)
						      fat_tab->aux_ant_sum_cck[i];
					main_ant_cnt = (u32)fat_tab->main_ant_cnt[i] + (u32)
						       fat_tab->main_ant_cnt_cck[i];
					aux_ant_cnt = (u32)fat_tab->aux_ant_cnt[i] + (u32)
						      fat_tab->aux_ant_cnt_cck[i];

					main_rssi = (main_ant_cnt != 0) ? (main_ant_sum / main_ant_cnt) : 0;
					aux_rssi = (aux_ant_cnt != 0) ? (aux_ant_sum / aux_ant_cnt) : 0;

					if (fat_tab->main_ant_cnt[i] <= 1 && fat_tab->main_ant_cnt_cck[i] >= 1)
						main_rssi = 0;

					if (fat_tab->aux_ant_cnt[i] <= 1 && fat_tab->aux_ant_cnt_cck[i] >= 1)
						aux_rssi = 0;
#endif
					if ((fat_tab->main_ant_cnt[i] != 0) || (fat_tab->aux_ant_cnt[i] != 0)) {
						main_ant_cnt = (u32)fat_tab->main_ant_cnt[i];
						aux_ant_cnt = (u32)fat_tab->aux_ant_cnt[i];
						main_rssi = (main_ant_cnt != 0) ? (fat_tab->main_ant_sum[i] / main_ant_cnt)
							    : 0;
						aux_rssi = (aux_ant_cnt != 0) ? (fat_tab->aux_ant_sum[i] / aux_ant_cnt) :
							   0;
						if (dm_swat_table->pre_antenna == MAIN_ANT) {
							target_ant = ((aux_ant_cnt > main_ant_cnt)
								      && (aux_rssi >= main_rssi)) ? AUX_ANT : dm_swat_table->pre_antenna;
						} else {
							target_ant = ((main_ant_cnt > aux_ant_cnt)
								      && (main_rssi >= aux_rssi)) ? MAIN_ANT : dm_swat_table->pre_antenna;
						}

					} else {	/*CCK only case*/
						main_ant_cnt = fat_tab->main_ant_cnt_cck[i];
						aux_ant_cnt = fat_tab->aux_ant_cnt_cck[i];
						main_rssi = (main_ant_cnt != 0) ? (fat_tab->main_ant_sum_cck[i] /
										   main_ant_cnt) : 0;
						aux_rssi = (aux_ant_cnt != 0) ? (fat_tab->aux_ant_sum_cck[i] / aux_ant_cnt)
							   : 0;
						target_ant = (main_rssi == aux_rssi) ? fat_tab->rx_idle_ant : ((
									main_rssi >= aux_rssi) ? MAIN_ANT :
								AUX_ANT); /*Use RSSI for CCK only case*/
					}
#if 0
					target_ant = (main_rssi == aux_rssi) ? dm_swat_table->pre_antenna : ((
								main_rssi >= aux_rssi) ? MAIN_ANT : AUX_ANT);
#endif
					local_max_rssi = (main_rssi >= aux_rssi) ? main_rssi : aux_rssi;
					local_min_rssi = (main_rssi >= aux_rssi) ? aux_rssi : main_rssi;

					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "***  CCK_counter_main = (( %d ))  , CCK_counter_aux= ((  %d ))\n",
						  fat_tab->main_ant_cnt_cck[i], fat_tab->aux_ant_cnt_cck[i]);
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "***  OFDM_counter_main = (( %d ))  , OFDM_counter_aux= ((  %d ))\n",
						  fat_tab->main_ant_cnt[i], fat_tab->aux_ant_cnt[i]);
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "***  Main_Cnt = (( %d ))  , main_rssi= ((  %d ))\n", main_ant_cnt,
						  main_rssi);
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "***  Aux_Cnt   = (( %d ))  , aux_rssi = ((  %d ))\n", aux_ant_cnt,
						  aux_rssi);
					PHYDM_DBG(dm, DBG_ANT_DIV, "*** MAC ID:[ %d ] , target_ant = (( %s ))\n",
						  i, (target_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

					/* 2 Select RX Idle Antenna */

					if (local_max_rssi != 0 && local_max_rssi < min_max_rssi) {
						rx_idle_ant = target_ant;
						min_max_rssi = local_max_rssi;
						PHYDM_DBG(dm, DBG_ANT_DIV, "*** local_max_rssi-local_min_rssi = ((%d))\n",
							  (local_max_rssi - local_min_rssi));

						if ((local_max_rssi - local_min_rssi) > 8) {
							if (local_min_rssi != 0)
								dm_swat_table->train_time_flag = 3;
							else {
								if (min_max_rssi > RSSI_CHECK_THRESHOLD)
									dm_swat_table->train_time_flag = 0;
								else
									dm_swat_table->train_time_flag = 3;
							}
						} else if ((local_max_rssi - local_min_rssi) > 5)
							dm_swat_table->train_time_flag = 2;
						else if ((local_max_rssi - local_min_rssi) > 2)
							dm_swat_table->train_time_flag = 1;
						else
							dm_swat_table->train_time_flag = 0;

					}

					/* 2 Select TX Antenna */
					if (target_ant == MAIN_ANT)
						fat_tab->antsel_a[i] = ANT1_2G;
					else
						fat_tab->antsel_a[i] = ANT2_2G;

				}
				phydm_antdiv_reset_statistic(dm, i);
				pkt_cnt_total += (main_ant_cnt + aux_ant_cnt);
			}

			if (dm_swat_table->is_sw_ant_div_by_ctrl_frame) {
				odm_s0s1_sw_ant_div_by_ctrl_frame(dm, SWAW_STEP_DETERMINE);
				is_by_ctrl_frame = true;
			}

			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "Control frame packet counter = %d, data frame packet counter = %llu\n",
				  dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame, pkt_cnt_total);

			if (min_max_rssi == 0xff
			    || ((pkt_cnt_total < (dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame >>
						  1)) && dm->phy_dbg_info.num_qry_beacon_pkt < 2)) {
				min_max_rssi = 0;
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "Check RSSI of control frame because min_max_rssi == 0xff\n");
				PHYDM_DBG(dm, DBG_ANT_DIV, "is_by_ctrl_frame = %d\n", is_by_ctrl_frame);

				if (is_by_ctrl_frame) {
					main_rssi = (fat_tab->main_ant_ctrl_frame_cnt != 0) ?
						    (fat_tab->main_ant_ctrl_frame_sum / fat_tab->main_ant_ctrl_frame_cnt) : 0;
					aux_rssi = (fat_tab->aux_ant_ctrl_frame_cnt != 0) ?
						   (fat_tab->aux_ant_ctrl_frame_sum / fat_tab->aux_ant_ctrl_frame_cnt) : 0;

					if (fat_tab->main_ant_ctrl_frame_cnt <= 1
					    && fat_tab->cck_ctrl_frame_cnt_main >= 1)
						main_rssi = 0;

					if (fat_tab->aux_ant_ctrl_frame_cnt <= 1
					    && fat_tab->cck_ctrl_frame_cnt_aux >= 1)
						aux_rssi = 0;

					if (main_rssi != 0 || aux_rssi != 0) {
						rx_idle_ant = (main_rssi == aux_rssi) ? dm_swat_table->pre_antenna : ((
									main_rssi >= aux_rssi) ? MAIN_ANT : AUX_ANT);
						local_max_rssi = (main_rssi >= aux_rssi) ? main_rssi : aux_rssi;
						local_min_rssi = (main_rssi >= aux_rssi) ? aux_rssi : main_rssi;

						if ((local_max_rssi - local_min_rssi) > 8)
							dm_swat_table->train_time_flag = 3;
						else if ((local_max_rssi - local_min_rssi) > 5)
							dm_swat_table->train_time_flag = 2;
						else if ((local_max_rssi - local_min_rssi) > 2)
							dm_swat_table->train_time_flag = 1;
						else
							dm_swat_table->train_time_flag = 0;

						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "Control frame: main_rssi = %d, aux_rssi = %d\n", main_rssi, aux_rssi);
						PHYDM_DBG(dm, DBG_ANT_DIV, "rx_idle_ant decided by control frame = %s\n",
							  (rx_idle_ant == MAIN_ANT ? "MAIN" : "AUX"));
					}
				}
			}

			fat_tab->min_max_rssi = min_max_rssi;
			dm_swat_table->try_flag = SWAW_STEP_PEEK;

			if (dm_swat_table->double_chk_flag == 1) {
				dm_swat_table->double_chk_flag = 0;

				if (fat_tab->min_max_rssi > RSSI_CHECK_THRESHOLD) {
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  " [Double check] min_max_rssi ((%d)) > %d again!!\n",
						  fat_tab->min_max_rssi, RSSI_CHECK_THRESHOLD);

					odm_update_rx_idle_ant(dm, rx_idle_ant);

					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[reset try_flag = 0] Training accomplished !!!]\n\n\n");
					return;
				} else {
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  " [Double check] min_max_rssi ((%d)) <= %d !!\n",
						  fat_tab->min_max_rssi, RSSI_CHECK_THRESHOLD);

					next_ant = (fat_tab->rx_idle_ant  == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
					dm_swat_table->try_flag = SWAW_STEP_PEEK;
					dm_swat_table->reset_idx = RSSI_CHECK_RESET_PERIOD;
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "[set try_flag=0]  Normal state:  Need to tryg again!!\n\n\n");
					return;
				}
			} else {
				if (fat_tab->min_max_rssi < RSSI_CHECK_THRESHOLD)
					dm_swat_table->reset_idx = RSSI_CHECK_RESET_PERIOD;

				dm_swat_table->pre_antenna = rx_idle_ant;
				odm_update_rx_idle_ant(dm, rx_idle_ant);
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "[reset try_flag = 0] Training accomplished !!!]\n\n\n");
				return;
			}

		}

	}

	/* 1 4.Change TRX antenna */

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "rssi_trying = (( %d )),    ant: (( %s )) >>> (( %s ))\n",
		  dm_swat_table->rssi_trying,
		  (fat_tab->rx_idle_ant  == MAIN_ANT ? "MAIN" : "AUX"),
		  (next_ant == MAIN_ANT ? "MAIN" : "AUX"));

	odm_update_rx_idle_ant(dm, next_ant);

	/* 1 5.Reset Statistics */

	fat_tab->rx_idle_ant  = next_ant;

	if (dm->support_ic_type == ODM_RTL8188F) {
		if (dm->support_interface == ODM_ITRF_SDIO) {
			ODM_delay_us(200);

			if (fat_tab->rx_idle_ant == MAIN_ANT) {
				fat_tab->main_ant_sum[0] = 0;
				fat_tab->main_ant_cnt[0] = 0;
				fat_tab->main_ant_sum_cck[0] = 0;
				fat_tab->main_ant_cnt_cck[0] = 0;
			} else {
				fat_tab->aux_ant_sum[0] = 0;
				fat_tab->aux_ant_cnt[0] = 0;
				fat_tab->aux_ant_sum_cck[0] = 0;
				fat_tab->aux_ant_cnt_cck[0] = 0;
			}
		}
	}

	/* 1 6.Set next timer   (Trying state) */

	PHYDM_DBG(dm, DBG_ANT_DIV, " Test ((%s)) ant for (( %d )) ms\n",
		  (next_ant == MAIN_ANT ? "MAIN" : "AUX"), dm_swat_table->train_time);
	odm_set_timer(dm, &dm_swat_table->phydm_sw_antenna_switch_timer,
		      dm_swat_table->train_time); /*ms*/
}


#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
void odm_sw_antdiv_callback(
	struct phydm_timer_list		*timer
)
{
	void		*adapter = (void *)timer->Adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));
	struct sw_antenna_switch			*dm_swat_table =
			&hal_data->DM_OutSrc.dm_swat_table;

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
#if USE_WORKITEM
	odm_schedule_work_item(&dm_swat_table->phydm_sw_antenna_switch_workitem);
#else
	{
		/* dbg_print("SW_antdiv_Callback"); */
		odm_s0s1_sw_ant_div(&hal_data->DM_OutSrc, SWAW_STEP_DETERMINE);
	}
#endif
#else
	odm_schedule_work_item(&dm_swat_table->phydm_sw_antenna_switch_workitem);
#endif
}
void odm_sw_antdiv_workitem_callback(
	void            *context
)
{
	void		*adapter = (void *)context;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	/* dbg_print("SW_antdiv_Workitem_Callback"); */
	odm_s0s1_sw_ant_div(&hal_data->DM_OutSrc, SWAW_STEP_DETERMINE);
}

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)

void odm_sw_antdiv_workitem_callback(
	void	*context
)
{
	void *
	adapter = (void *)context;
	HAL_DATA_TYPE
	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

	/*dbg_print("SW_antdiv_Workitem_Callback");*/
	odm_s0s1_sw_ant_div(&hal_data->odmpriv, SWAW_STEP_DETERMINE);
}

void odm_sw_antdiv_callback(void *function_context)
{
	struct dm_struct	*dm = (struct dm_struct *)function_context;
	void	*padapter = dm->adapter;
	if (*(dm->is_net_closed) == true)
		return;

#if 0 /* Can't do I/O in timer callback*/
	odm_s0s1_sw_ant_div(dm, SWAW_STEP_DETERMINE);
#else
	rtw_run_in_thread_cmd(padapter, odm_sw_antdiv_workitem_callback, padapter);
#endif
}


#endif

void odm_s0s1_sw_ant_div_by_ctrl_frame(
	void			*dm_void,
	u8			step
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct sw_antenna_switch	*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	switch (step) {
	case SWAW_STEP_PEEK:
		dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame = 0;
		dm_swat_table->is_sw_ant_div_by_ctrl_frame = true;
		fat_tab->main_ant_ctrl_frame_cnt = 0;
		fat_tab->aux_ant_ctrl_frame_cnt = 0;
		fat_tab->main_ant_ctrl_frame_sum = 0;
		fat_tab->aux_ant_ctrl_frame_sum = 0;
		fat_tab->cck_ctrl_frame_cnt_main = 0;
		fat_tab->cck_ctrl_frame_cnt_aux = 0;
		fat_tab->ofdm_ctrl_frame_cnt_main = 0;
		fat_tab->ofdm_ctrl_frame_cnt_aux = 0;
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "odm_S0S1_SwAntDivForAPMode(): Start peek and reset counter\n");
		break;
	case SWAW_STEP_DETERMINE:
		dm_swat_table->is_sw_ant_div_by_ctrl_frame = false;
		PHYDM_DBG(dm, DBG_ANT_DIV, "odm_S0S1_SwAntDivForAPMode(): Stop peek\n");
		break;
	default:
		dm_swat_table->is_sw_ant_div_by_ctrl_frame = false;
		break;
	}
}

void odm_antsel_statistics_of_ctrl_frame(
	void			*dm_void,
	u8			antsel_tr_mux,
	u32			rx_pwdb_all

)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	if (antsel_tr_mux == ANT1_2G) {
		fat_tab->main_ant_ctrl_frame_sum += rx_pwdb_all;
		fat_tab->main_ant_ctrl_frame_cnt++;
	} else {
		fat_tab->aux_ant_ctrl_frame_sum += rx_pwdb_all;
		fat_tab->aux_ant_ctrl_frame_cnt++;
	}
}

void odm_s0s1_sw_ant_div_by_ctrl_frame_process_rssi(
	void			*dm_void,
	void			*phy_info_void,
	void			*pkt_info_void
	/*	struct phydm_phyinfo_struct*		phy_info, */
	/*	struct phydm_perpkt_info_struct*		pktinfo */
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_phyinfo_struct	*phy_info = (struct phydm_phyinfo_struct *)
			phy_info_void;
	struct phydm_perpkt_info_struct	*pktinfo = (struct phydm_perpkt_info_struct
			*)pkt_info_void;
	struct sw_antenna_switch	*dm_swat_table = &dm->dm_swat_table;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	if (!(dm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (dm->ant_div_type != S0S1_SW_ANTDIV)
		return;

	/* In try state */
	if (!dm_swat_table->is_sw_ant_div_by_ctrl_frame)
		return;

	/* No HW error and match receiver address */
	if (!pktinfo->is_to_self)
		return;

	dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame++;

	if (pktinfo->is_cck_rate) {
		fat_tab->antsel_rx_keep_0 = (fat_tab->rx_idle_ant == MAIN_ANT) ? ANT1_2G :
					    ANT2_2G;

		if (fat_tab->antsel_rx_keep_0 == ANT1_2G)
			fat_tab->cck_ctrl_frame_cnt_main++;
		else
			fat_tab->cck_ctrl_frame_cnt_aux++;

		odm_antsel_statistics_of_ctrl_frame(dm, fat_tab->antsel_rx_keep_0,
						    phy_info->rx_mimo_signal_strength[RF_PATH_A]);
	} else {
		fat_tab->antsel_rx_keep_0 = (fat_tab->rx_idle_ant == MAIN_ANT) ? ANT1_2G :
					    ANT2_2G;

		if (fat_tab->antsel_rx_keep_0 == ANT1_2G)
			fat_tab->ofdm_ctrl_frame_cnt_main++;
		else
			fat_tab->ofdm_ctrl_frame_cnt_aux++;

		odm_antsel_statistics_of_ctrl_frame(dm, fat_tab->antsel_rx_keep_0,
						    phy_info->rx_pwdb_all);
	}
}

#endif /* #if (RTL8723B_SUPPORT == 1) || (RTL8821A_SUPPORT == 1) */




void odm_set_next_mac_addr_target(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	struct cmn_sta_info	*entry;
	u32			value32, i;

	PHYDM_DBG(dm, DBG_ANT_DIV, "odm_set_next_mac_addr_target() ==>\n");

	if (dm->is_linked) {
		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
			if ((fat_tab->train_idx + 1) == ODM_ASSOCIATE_ENTRY_NUM)
				fat_tab->train_idx = 0;
			else
				fat_tab->train_idx++;

			entry = dm->phydm_sta_info[fat_tab->train_idx];

			if (is_sta_active(entry)) {
				/*Match MAC ADDR*/
				value32 = (entry->mac_addr[5] << 8) | entry->mac_addr[4];

				odm_set_mac_reg(dm, 0x7b4, 0xFFFF, value32);/*0x7b4~0x7b5*/

				value32 = (entry->mac_addr[3] << 24) | (entry->mac_addr[2] << 16) |
					  (entry->mac_addr[1] << 8) | entry->mac_addr[0];

				odm_set_mac_reg(dm, 0x7b0, MASKDWORD, value32);/*0x7b0~0x7b3*/

				PHYDM_DBG(dm, DBG_ANT_DIV, "fat_tab->train_idx=%d\n", fat_tab->train_idx);

				PHYDM_DBG(dm, DBG_ANT_DIV, "Training MAC addr = %x:%x:%x:%x:%x:%x\n",
					  entry->mac_addr[5], entry->mac_addr[4], entry->mac_addr[3],
					  entry->mac_addr[2], entry->mac_addr[1], entry->mac_addr[0]);

				break;
			}
		}
	}

}

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))

void odm_fast_ant_training(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	u32	max_rssi_path_a = 0, pckcnt_path_a = 0;
	u8	i, target_ant_path_a = 0;
	boolean	is_pkt_filter_macth_path_a = false;
#if (RTL8192E_SUPPORT == 1)
	u32	max_rssi_path_b = 0, pckcnt_path_b = 0;
	u8	target_ant_path_b = 0;
	boolean	is_pkt_filter_macth_path_b = false;
#endif


	if (!dm->is_linked) { /* is_linked==False */
		PHYDM_DBG(dm, DBG_ANT_DIV, "[No Link!!!]\n");

		if (fat_tab->is_become_linked == true) {
			odm_ant_div_on_off(dm, ANTDIV_OFF);
			phydm_fast_training_enable(dm, FAT_OFF);
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
			fat_tab->is_become_linked = dm->is_linked;
		}
		return;
	} else {
		if (fat_tab->is_become_linked == false) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "[Linked!!!]\n");
			fat_tab->is_become_linked = dm->is_linked;
		}
	}

	if (*fat_tab->p_force_tx_ant_by_desc == false) {
		if (dm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);
	}


	if (dm->support_ic_type == ODM_RTL8188E)
		odm_set_bb_reg(dm, 0x864, BIT(2) | BIT(1) | BIT(0),
			       ((dm->fat_comb_a) - 1));
#if (RTL8192E_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8192E) {
		odm_set_bb_reg(dm, 0xB38, BIT(2) | BIT(1) | BIT(0),
			       ((dm->fat_comb_a) -
				1));	   /* path-A  */ /* ant combination=regB38[2:0]+1 */
		odm_set_bb_reg(dm, 0xB38, BIT(18) | BIT(17) | BIT(16),
			       ((dm->fat_comb_b) -
				1));  /* path-B  */ /* ant combination=regB38[18:16]+1 */
	}
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV, "==>odm_fast_ant_training()\n");

	/* 1 TRAINING STATE */
	if (fat_tab->fat_state == FAT_TRAINING_STATE) {
		/* 2 Caculate RSSI per Antenna */

		/* 3 [path-A]--------------------------- */
		for (i = 0; i < (dm->fat_comb_a); i++) { /* i : antenna index */
			if (fat_tab->ant_rssi_cnt[i] == 0)
				fat_tab->ant_ave_rssi[i] = 0;
			else {
				fat_tab->ant_ave_rssi[i] = fat_tab->ant_sum_rssi[i] /
							   fat_tab->ant_rssi_cnt[i];
				is_pkt_filter_macth_path_a = true;
			}

			if (fat_tab->ant_ave_rssi[i] > max_rssi_path_a) {
				max_rssi_path_a = fat_tab->ant_ave_rssi[i];
				pckcnt_path_a = fat_tab->ant_rssi_cnt[i];
				target_ant_path_a =  i ;
			} else if (fat_tab->ant_ave_rssi[i] == max_rssi_path_a) {
				if ((fat_tab->ant_rssi_cnt[i])   >   pckcnt_path_a) {
					max_rssi_path_a = fat_tab->ant_ave_rssi[i];
					pckcnt_path_a = fat_tab->ant_rssi_cnt[i];
					target_ant_path_a = i ;
				}
			}

			PHYDM_DBG("*** ant-index : [ %d ],      counter = (( %d )),     Avg RSSI = (( %d ))\n",
				  i, fat_tab->ant_rssi_cnt[i],  fat_tab->ant_ave_rssi[i]);
		}


#if 0
#if (RTL8192E_SUPPORT == 1)
		/* 3 [path-B]--------------------------- */
		for (i = 0; i < (dm->fat_comb_b); i++) {
			if (fat_tab->antRSSIcnt_pathB[i] == 0)
				fat_tab->antAveRSSI_pathB[i] = 0;
			else { /*  (ant_rssi_cnt[i] != 0) */
				fat_tab->antAveRSSI_pathB[i] = fat_tab->antSumRSSI_pathB[i] /
							       fat_tab->antRSSIcnt_pathB[i];
				is_pkt_filter_macth_path_b = true;
			}
			if (fat_tab->antAveRSSI_pathB[i] > max_rssi_path_b) {
				max_rssi_path_b = fat_tab->antAveRSSI_pathB[i];
				pckcnt_path_b = fat_tab->antRSSIcnt_pathB[i];
				target_ant_path_b = (u8) i;
			}
			if (fat_tab->antAveRSSI_pathB[i] == max_rssi_path_b) {
				if (fat_tab->antRSSIcnt_pathB > pckcnt_path_b) {
					max_rssi_path_b = fat_tab->antAveRSSI_pathB[i];
					target_ant_path_b = (u8) i;
				}
			}
			if (dm->fat_print_rssi == 1) {
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "***{path-B}: Sum RSSI[%d] = (( %d )),      cnt RSSI [%d] = (( %d )),     Avg RSSI[%d] = (( %d ))\n",
					  i, fat_tab->antSumRSSI_pathB[i], i, fat_tab->antRSSIcnt_pathB[i], i,
					  fat_tab->antAveRSSI_pathB[i]);
			}
		}
#endif
#endif

		/* 1 DECISION STATE */

		/* 2 Select TRX Antenna */

		phydm_fast_training_enable(dm, FAT_OFF);

		/* 3 [path-A]--------------------------- */
		if (is_pkt_filter_macth_path_a  == false) {
			/* PHYDM_DBG(dm,DBG_ANT_DIV, "{path-A}: None Packet is matched\n"); */
			PHYDM_DBG(dm, DBG_ANT_DIV, "{path-A}: None Packet is matched\n");
			odm_ant_div_on_off(dm, ANTDIV_OFF);
		} else {
			PHYDM_DBG("target_ant_path_a = (( %d )) , max_rssi_path_a = (( %d ))\n",
				  target_ant_path_a, max_rssi_path_a);

			/* 3 [ update RX-optional ant ]        Default RX is Omni, Optional RX is the best decision by FAT */
			if (dm->support_ic_type == ODM_RTL8188E)
				odm_set_bb_reg(dm, 0x864, BIT(8) | BIT(7) | BIT(6), target_ant_path_a);
			else if (dm->support_ic_type == ODM_RTL8192E)
				odm_set_bb_reg(dm, 0xB38, BIT(8) | BIT(7) | BIT(6),
					       target_ant_path_a); /* Optional RX [pth-A] */

			/* 3 [ update TX ant ] */
			odm_update_tx_ant(dm, target_ant_path_a, (fat_tab->train_idx));

			if (target_ant_path_a == 0)
				odm_ant_div_on_off(dm, ANTDIV_OFF);
		}
#if 0
#if (RTL8192E_SUPPORT == 1)
		/* 3 [path-B]--------------------------- */
		if (is_pkt_filter_macth_path_b == false) {
			if (dm->fat_print_rssi == 1)
				PHYDM_DBG(dm, DBG_ANT_DIV, "***[%d]{path-B}: None Packet is matched\n\n\n",
					  __LINE__);
		} else {
			if (dm->fat_print_rssi == 1) {
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  " ***target_ant_path_b = (( %d )) *** max_rssi = (( %d ))***\n\n\n",
					  target_ant_path_b, max_rssi_path_b);
			}
			odm_set_bb_reg(dm, 0xB38, BIT(21) | BIT20 | BIT19,
				       target_ant_path_b);	/* Default RX is Omni, Optional RX is the best decision by FAT */
			odm_set_bb_reg(dm, 0x80c, BIT(21),
				       1); /* Reg80c[21]=1'b1		//from TX Info */

			fat_tab->antsel_pathB[fat_tab->train_idx] = target_ant_path_b;
		}
#endif
#endif

		/* 2 Reset counter */
		for (i = 0; i < (dm->fat_comb_a); i++) {
			fat_tab->ant_sum_rssi[i] = 0;
			fat_tab->ant_rssi_cnt[i] = 0;
		}
		/*
		#if (RTL8192E_SUPPORT == 1)
		for(i=0; i<=(dm->fat_comb_b); i++)
		{
			fat_tab->antSumRSSI_pathB[i] = 0;
			fat_tab->antRSSIcnt_pathB[i] = 0;
		}
		#endif
		*/

		fat_tab->fat_state = FAT_PREPARE_STATE;
		return;
	}

	/* 1 NORMAL STATE */
	if (fat_tab->fat_state == FAT_PREPARE_STATE) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ Start Prepare state ]\n");

		odm_set_next_mac_addr_target(dm);

		/* 2 Prepare Training */
		fat_tab->fat_state = FAT_TRAINING_STATE;
		phydm_fast_training_enable(dm, FAT_ON);
		odm_ant_div_on_off(dm, ANTDIV_ON);		/* enable HW AntDiv */
		PHYDM_DBG(dm, DBG_ANT_DIV, "[Start Training state]\n");

		odm_set_timer(dm, &dm->fast_ant_training_timer, dm->antdiv_intvl); /* ms */
	}

}

void odm_fast_ant_training_callback(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	if (*(dm->is_net_closed) == true)
		return;
#endif

#if USE_WORKITEM
	odm_schedule_work_item(&dm->fast_ant_training_workitem);
#else
	PHYDM_DBG(dm, DBG_ANT_DIV, "******odm_fast_ant_training_callback******\n");
	odm_fast_ant_training(dm);
#endif
}

void odm_fast_ant_training_work_item_callback(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "******odm_fast_ant_training_work_item_callback******\n");
	odm_fast_ant_training(dm);
}

#endif

void odm_ant_div_init(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
	struct sw_antenna_switch			*dm_swat_table = &dm->dm_swat_table;


	if (!(dm->support_ability & ODM_BB_ANT_DIV)) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[Return!!!]   Not Support Antenna Diversity Function\n");
		return;
	}
	/* --- */
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	if (fat_tab->ant_div_2g_5g == ODM_ANTDIV_2G) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[2G AntDiv Init]: Only Support 2G Antenna Diversity Function\n");
		if (!(dm->support_ic_type & ODM_ANTDIV_2G_SUPPORT_IC))
			return;
	} else	if (fat_tab->ant_div_2g_5g == ODM_ANTDIV_5G) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[5G AntDiv Init]: Only Support 5G Antenna Diversity Function\n");
		if (!(dm->support_ic_type & ODM_ANTDIV_5G_SUPPORT_IC))
			return;
	} else	if (fat_tab->ant_div_2g_5g == (ODM_ANTDIV_2G | ODM_ANTDIV_5G))
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[2G & 5G AntDiv Init]:Support Both 2G & 5G Antenna Diversity Function\n");

#endif
	/* --- */

	/* 2 [--General---] */
	dm->antdiv_period = 0;

	fat_tab->is_become_linked = false;
	fat_tab->ant_div_on_off = 0xff;

	/* 3       -   AP   - */
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

#if (BEAMFORMING_SUPPORT == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
	odm_bdc_init(dm);
#endif
#endif

	/* 3     -   WIN   - */
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	dm_swat_table->ant_5g = MAIN_ANT;
	dm_swat_table->ant_2g = MAIN_ANT;
#endif

	/* 2 [---Set MAIN_ANT as default antenna if Auto-ant enable---] */
	odm_ant_div_on_off(dm, ANTDIV_OFF);

	dm->ant_type = ODM_AUTO_ANT;

	fat_tab->rx_idle_ant =
		0xff; /*to make RX-idle-antenna will be updated absolutly*/
	odm_update_rx_idle_ant(dm, MAIN_ANT);
	phydm_keep_rx_ack_ant_by_tx_ant_time(dm,
					     0);  /* Timming issue: keep Rx ant after tx for ACK ( 5 x 3.2 mu = 16mu sec)*/

	/* 2 [---Set TX Antenna---] */
	if (fat_tab->p_force_tx_ant_by_desc == NULL) {
		fat_tab->force_tx_ant_by_desc = 0;
		fat_tab->p_force_tx_ant_by_desc = &fat_tab->force_tx_ant_by_desc;
	}
	PHYDM_DBG(dm, DBG_ANT_DIV, "p_force_tx_ant_by_desc = %d\n",
		  *fat_tab->p_force_tx_ant_by_desc);

	if (*fat_tab->p_force_tx_ant_by_desc == true)
		odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);
	else
		odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);


	/* 2 [--88E---] */
	if (dm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		/* dm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* dm->ant_div_type = CG_TRX_HW_ANTDIV; */
		/* dm->ant_div_type = CG_TRX_SMART_ANTDIV; */

		if ((dm->ant_div_type != CGCS_RX_HW_ANTDIV)
		    && (dm->ant_div_type != CG_TRX_HW_ANTDIV)
		    && (dm->ant_div_type != CG_TRX_SMART_ANTDIV)) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!]  88E Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (dm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_rx_hw_ant_div_init_88e(dm);
		else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_88e(dm);
#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (dm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_smart_hw_ant_div_init_88e(dm);
#endif
#endif
	}

	/* 2 [--92E---] */
#if (RTL8192E_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8192E) {
		/* dm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* dm->ant_div_type = CG_TRX_HW_ANTDIV; */
		/* dm->ant_div_type = CG_TRX_SMART_ANTDIV; */

		if ((dm->ant_div_type != CGCS_RX_HW_ANTDIV)
		    && (dm->ant_div_type != CG_TRX_HW_ANTDIV)
		    && (dm->ant_div_type != CG_TRX_SMART_ANTDIV)) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!]  8192E Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (dm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_rx_hw_ant_div_init_92e(dm);
		else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_92e(dm);
#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (dm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_smart_hw_ant_div_init_92e(dm);
#endif

	}
#endif

	/* 2 [--8723B---] */
#if (RTL8723B_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8723B) {
		dm->ant_div_type = S0S1_SW_ANTDIV;
		/* dm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (dm->ant_div_type != S0S1_SW_ANTDIV
		    && dm->ant_div_type != CG_TRX_HW_ANTDIV) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!] 8723B  Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (dm->ant_div_type == S0S1_SW_ANTDIV)
			odm_s0s1_sw_ant_div_init_8723b(dm);
		else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_8723b(dm);
	}
#endif
	/*2 [--8723D---]*/
#if (RTL8723D_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8723D) {
		if (fat_tab->p_default_s0_s1 == NULL) {
			fat_tab->default_s0_s1 = 1;
			fat_tab->p_default_s0_s1 = &fat_tab->default_s0_s1;
		}
		PHYDM_DBG(dm, DBG_ANT_DIV, "default_s0_s1 = %d\n",
			  *fat_tab->p_default_s0_s1);

		if (*fat_tab->p_default_s0_s1 == true)
			odm_update_rx_idle_ant(dm, MAIN_ANT);
		else
			odm_update_rx_idle_ant(dm, AUX_ANT);

		if (dm->ant_div_type == S0S1_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_8723d(dm);
		else {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!] 8723D  Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

	}
#endif
	/* 2 [--8811A 8821A---] */
#if (RTL8821A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8821) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		dm->ant_div_type = HL_SW_SMART_ANT_TYPE1;

		if (dm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {
			odm_trx_hw_ant_div_init_8821a(dm);
			phydm_hl_smart_ant_type1_init_8821a(dm);
		} else
#endif
		{
#ifdef ODM_CONFIG_BT_COEXIST
			dm->ant_div_type = S0S1_SW_ANTDIV;
#else
			dm->ant_div_type = CG_TRX_HW_ANTDIV;
#endif

			if (dm->ant_div_type != CG_TRX_HW_ANTDIV
			    && dm->ant_div_type != S0S1_SW_ANTDIV) {
				PHYDM_DBG(dm, DBG_ANT_DIV,
					  "[Return!!!] 8821A & 8811A  Not Supprrt This AntDiv type\n");
				dm->support_ability &= ~(ODM_BB_ANT_DIV);
				return;
			}
			if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_trx_hw_ant_div_init_8821a(dm);
			else if (dm->ant_div_type == S0S1_SW_ANTDIV)
				odm_s0s1_sw_ant_div_init_8821a(dm);
		}
	}
#endif

	/* 2 [--8821C---] */
#if (RTL8821C_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8821C) {
		dm->ant_div_type = S0S1_SW_ANTDIV;
		if (dm->ant_div_type != S0S1_SW_ANTDIV) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!] 8821C  Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}
		phydm_s0s1_sw_ant_div_init_8821c(dm);
		odm_trx_hw_ant_div_init_8821c(dm);
	}
#endif

	/* 2 [--8881A---] */
#if (RTL8881A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8881A) {
		/* dm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* dm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (dm->ant_div_type == CG_TRX_HW_ANTDIV) {
			odm_trx_hw_ant_div_init_8881a(dm);
			/**/
		} else {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!] 8881A  Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		odm_trx_hw_ant_div_init_8881a(dm);
	}
#endif

	/* 2 [--8812---] */
#if (RTL8812A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8812) {
		/* dm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (dm->ant_div_type != CG_TRX_HW_ANTDIV) {
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[Return!!!] 8812A  Not Supprrt This AntDiv type\n");
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}
		odm_trx_hw_ant_div_init_8812a(dm);
	}
#endif

	/*[--8188F---]*/
#if (RTL8188F_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8188F) {
		dm->ant_div_type = S0S1_SW_ANTDIV;
		odm_s0s1_sw_ant_div_init_8188f(dm);
	}
#endif

	/*[--8822B---]*/
#if (RTL8822B_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8822B) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2
		dm->ant_div_type = HL_SW_SMART_ANT_TYPE2;

		if (dm->ant_div_type == HL_SW_SMART_ANT_TYPE2)
			phydm_hl_smart_ant_type2_init_8822b(dm);
#endif
	}
#endif

	/*
	PHYDM_DBG(dm, DBG_ANT_DIV, "*** support_ic_type=[%lu]\n",dm->support_ic_type);
	PHYDM_DBG(dm, DBG_ANT_DIV, "*** AntDiv support_ability=[%lu]\n",(dm->support_ability & ODM_BB_ANT_DIV)>>6);
	PHYDM_DBG(dm, DBG_ANT_DIV, "*** AntDiv type=[%d]\n",dm->ant_div_type);
	*/
}

void odm_ant_div(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	void		*adapter	= dm->adapter;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
#if (defined(CONFIG_HL_SMART_ANTENNA))
	struct smt_ant_honbo			*sat_tab = &dm->dm_sat_table;
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV

	if (dm->is_linked) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "tp_active_occur=((%d)), evm_method_enable=((%d))\n",
			  dm->tp_active_occur, fat_tab->evm_method_enable);

		if ((dm->tp_active_occur == 1) && (fat_tab->evm_method_enable == 1)) {
			fat_tab->idx_ant_div_counter_5g = dm->antdiv_period;
			fat_tab->idx_ant_div_counter_2g = dm->antdiv_period;
		}
	}
#endif

	if (*dm->band_type == ODM_BAND_5G) {
		if (fat_tab->idx_ant_div_counter_5g <  dm->antdiv_period) {
			fat_tab->idx_ant_div_counter_5g++;
			return;
		} else
			fat_tab->idx_ant_div_counter_5g = 0;
	} else	if (*dm->band_type == ODM_BAND_2_4G) {
		if (fat_tab->idx_ant_div_counter_2g <  dm->antdiv_period) {
			fat_tab->idx_ant_div_counter_2g++;
			return;
		} else
			fat_tab->idx_ant_div_counter_2g = 0;
	}

	/* ---------- */

	/* ---------- */
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN || DM_ODM_SUPPORT_TYPE == ODM_CE)

	if (fat_tab->enable_ctrl_frame_antdiv) {
		if ((dm->data_frame_num <= 10) && (dm->is_linked))
			fat_tab->use_ctrl_frame_antdiv = 1;
		else
			fat_tab->use_ctrl_frame_antdiv = 0;

		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "use_ctrl_frame_antdiv = (( %d )), data_frame_num = (( %d ))\n",
			  fat_tab->use_ctrl_frame_antdiv, dm->data_frame_num);
		dm->data_frame_num = 0;
	}

	{
#if (BEAMFORMING_SUPPORT == 1)

		enum beamforming_cap		beamform_cap = phydm_get_beamform_cap(dm);
		PHYDM_DBG(dm, DBG_ANT_DIV, "is_bt_continuous_turn = ((%d))\n",
			  dm->is_bt_continuous_turn);
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ AntDiv Beam Cap ]   cap= ((%d))\n",
			  beamform_cap);
		if (!dm->is_bt_continuous_turn) {
			if ((beamform_cap & BEAMFORMEE_CAP)
			    && (!(*fat_tab->is_no_csi_feedback))) { /* BFmee On  &&   Div On->Div Off */
				PHYDM_DBG(dm, DBG_ANT_DIV, "[ AntDiv : OFF ]   BFmee ==1; cap= ((%d))\n",
					  beamform_cap);
				PHYDM_DBG(dm, DBG_ANT_DIV, "[ AntDiv BF]   is_no_csi_feedback= ((%d))\n",
					  *(fat_tab->is_no_csi_feedback));
				if (fat_tab->fix_ant_bfee == 0) {
					odm_ant_div_on_off(dm, ANTDIV_OFF);
					fat_tab->fix_ant_bfee = 1;
				}
				return;
			} else { /* BFmee Off   &&   Div Off->Div On */
				if ((fat_tab->fix_ant_bfee == 1)  &&  dm->is_linked) {
					PHYDM_DBG(dm, DBG_ANT_DIV, "[ AntDiv : ON ]   BFmee ==0; cap=((%d))\n",
						  beamform_cap);
					PHYDM_DBG(dm, DBG_ANT_DIV, "[ AntDiv BF]   is_no_csi_feedback= ((%d))\n",
						  *(fat_tab->is_no_csi_feedback));
					if (dm->ant_div_type != S0S1_SW_ANTDIV)
						odm_ant_div_on_off(dm, ANTDIV_ON);

					fat_tab->fix_ant_bfee = 0;
				}
			}
		} else {
			odm_ant_div_on_off(dm, ANTDIV_ON);
		}
#endif
	}
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)
	/* ----------just for fool proof */

	if (dm->antdiv_rssi)
		dm->debug_components |= DBG_ANT_DIV;
	else
		dm->debug_components &= ~DBG_ANT_DIV;

	if (fat_tab->ant_div_2g_5g == ODM_ANTDIV_2G) {
		/* PHYDM_DBG(dm, DBG_ANT_DIV,"[ 2G AntDiv Running ]\n"); */
		if (!(dm->support_ic_type & ODM_ANTDIV_2G_SUPPORT_IC))
			return;
	} else if (fat_tab->ant_div_2g_5g == ODM_ANTDIV_5G) {
		/* PHYDM_DBG(dm, DBG_ANT_DIV,"[ 5G AntDiv Running ]\n"); */
		if (!(dm->support_ic_type & ODM_ANTDIV_5G_SUPPORT_IC))
			return;
	}
	/* else 	if(fat_tab->ant_div_2g_5g == (ODM_ANTDIV_2G|ODM_ANTDIV_5G)) */
	/* { */
	/* PHYDM_DBG(dm, DBG_ANT_DIV,"[ 2G & 5G AntDiv Running ]\n"); */
	/* } */
#endif

	/* ---------- */

	if (dm->antdiv_select == 1)
		dm->ant_type = ODM_FIX_MAIN_ANT;
	else if (dm->antdiv_select == 2)
		dm->ant_type = ODM_FIX_AUX_ANT;
	else { /* if (dm->antdiv_select==0) */
		dm->ant_type = ODM_AUTO_ANT;

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		/*Stop Antenna diversity for CMW500 testing case*/
		if (dm->consecutive_idlel_time >= 10) {
			dm->ant_type = ODM_FIX_MAIN_ANT;
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "[AntDiv: OFF] No TP case, consecutive_idlel_time=((%d))\n",
				  dm->consecutive_idlel_time);
		}
#endif
	}

	/* PHYDM_DBG(dm, DBG_ANT_DIV,"ant_type= (( %d )) , pre_ant_type= (( %d ))\n",dm->ant_type,dm->pre_ant_type); */

	if (dm->ant_type != ODM_AUTO_ANT) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "Fix Antenna at (( %s ))\n",
			  (dm->ant_type == ODM_FIX_MAIN_ANT) ? "MAIN" : "AUX");

		if (dm->ant_type != dm->pre_ant_type) {
			odm_ant_div_on_off(dm, ANTDIV_OFF);
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_REG);

			if (dm->ant_type == ODM_FIX_MAIN_ANT)
				odm_update_rx_idle_ant(dm, MAIN_ANT);
			else if (dm->ant_type == ODM_FIX_AUX_ANT)
				odm_update_rx_idle_ant(dm, AUX_ANT);
		}
		dm->pre_ant_type = dm->ant_type;
		return;
	} else {
		if (dm->ant_type != dm->pre_ant_type) {
			odm_ant_div_on_off(dm, ANTDIV_ON);
			odm_tx_by_tx_desc_or_reg(dm, TX_BY_DESC);
		}
		dm->pre_ant_type = dm->ant_type;
	}


	/* 3 ----------------------------------------------------------------------------------------------------------- */
	/* 2 [--88E---] */
	if (dm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		if (dm->ant_div_type == CG_TRX_HW_ANTDIV
		    || dm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_hw_ant_div(dm);

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (dm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_fast_ant_training(dm);
#endif

#endif

	}
	/* 2 [--92E---] */
#if (RTL8192E_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8192E) {
		if (dm->ant_div_type == CGCS_RX_HW_ANTDIV
		    || dm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_hw_ant_div(dm);

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (dm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_fast_ant_training(dm);
#endif

	}
#endif

#if (RTL8723B_SUPPORT == 1)
	/* 2 [--8723B---] */
	else if (dm->support_ic_type == ODM_RTL8723B) {
		if (phydm_is_bt_enable_8723b(dm)) {
			PHYDM_DBG(dm, DBG_ANT_DIV, "[BT is enable!!!]\n");
			if (fat_tab->is_become_linked == true) {
				PHYDM_DBG(dm, DBG_ANT_DIV, "Set REG 948[9:6]=0x0\n");
				if (dm->support_ic_type == ODM_RTL8723B)
					odm_set_bb_reg(dm, 0x948, BIT(9) | BIT(8) | BIT(7) | BIT(6), 0x0);

				fat_tab->is_become_linked = false;
			}
		} else {
			if (dm->ant_div_type == S0S1_SW_ANTDIV) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
				odm_s0s1_sw_ant_div(dm, SWAW_STEP_PEEK);
#endif
			} else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_hw_ant_div(dm);
		}
	}
#endif
	/*8723D*/
#if (RTL8723D_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8723D) {
		odm_hw_ant_div(dm);
		/**/
	}
#endif

	/* 2 [--8821A---] */
#if (RTL8821A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8821) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		if (dm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {
			if (sat_tab->fix_beam_pattern_en != 0) {
				PHYDM_DBG(dm, DBG_ANT_DIV, " [ SmartAnt ] Fix SmartAnt Pattern = 0x%x\n",
					  sat_tab->fix_beam_pattern_codeword);
				/*return;*/
			} else {
				/*PHYDM_DBG(dm, DBG_ANT_DIV, "[ SmartAnt ] ant_div_type = HL_SW_SMART_ANT_TYPE1\n");*/
				odm_fast_ant_training_hl_smart_antenna_type1(dm);
			}

		} else
#endif
		{
#ifdef ODM_CONFIG_BT_COEXIST
			if (!dm->bt_info_table.is_bt_enabled) { /*BT disabled*/
				if (dm->ant_div_type == S0S1_SW_ANTDIV) {
					dm->ant_div_type = CG_TRX_HW_ANTDIV;
					PHYDM_DBG(dm, DBG_ANT_DIV, " [S0S1_SW_ANTDIV]  ->  [CG_TRX_HW_ANTDIV]\n");
					/*odm_set_bb_reg(dm, 0x8D4, BIT24, 1); */
					if (fat_tab->is_become_linked == true)
						odm_ant_div_on_off(dm, ANTDIV_ON);
				}

			} else { /*BT enabled*/

				if (dm->ant_div_type == CG_TRX_HW_ANTDIV) {
					dm->ant_div_type = S0S1_SW_ANTDIV;
					PHYDM_DBG(dm, DBG_ANT_DIV, " [CG_TRX_HW_ANTDIV]  ->  [S0S1_SW_ANTDIV]\n");
					/*odm_set_bb_reg(dm, 0x8D4, BIT24, 0);*/
					odm_ant_div_on_off(dm, ANTDIV_OFF);
				}
			}
#endif

			if (dm->ant_div_type == S0S1_SW_ANTDIV) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
				odm_s0s1_sw_ant_div(dm, SWAW_STEP_PEEK);
#endif
			} else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_hw_ant_div(dm);
		}
	}
#endif

	/* 2 [--8821C---] */
#if (RTL8821C_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8821C) {
		if (!dm->is_bt_continuous_turn) {
			dm->ant_div_type = S0S1_SW_ANTDIV;
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "is_bt_continuous_turn = ((%d))   ==> SW AntDiv\n",
				  dm->is_bt_continuous_turn);

		} else {
			dm->ant_div_type = CG_TRX_HW_ANTDIV;
			PHYDM_DBG(dm, DBG_ANT_DIV,
				  "is_bt_continuous_turn = ((%d))   ==> HW AntDiv\n",
				  dm->is_bt_continuous_turn);
			odm_ant_div_on_off(dm, ANTDIV_ON);
		}

		if (dm->ant_div_type == S0S1_SW_ANTDIV) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
			odm_s0s1_sw_ant_div(dm, SWAW_STEP_PEEK);
#endif
		} else if (dm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_hw_ant_div(dm);
	}
#endif

	/* 2 [--8881A---] */
#if (RTL8881A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8881A)
		odm_hw_ant_div(dm);
#endif

	/* 2 [--8812A---] */
#if (RTL8812A_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8812)
		odm_hw_ant_div(dm);
#endif

#if (RTL8188F_SUPPORT == 1)
	/* [--8188F---]*/
	else if (dm->support_ic_type == ODM_RTL8188F)	{
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_s0s1_sw_ant_div(dm, SWAW_STEP_PEEK);
#endif
	}
#endif

	/* [--8822B---]*/
#if (RTL8822B_SUPPORT == 1)
	else if (dm->support_ic_type == ODM_RTL8822B) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2
		if (dm->ant_div_type == HL_SW_SMART_ANT_TYPE2) {
			if (sat_tab->fix_beam_pattern_en != 0)
				PHYDM_DBG(dm, DBG_ANT_DIV, " [ SmartAnt ] Fix SmartAnt Pattern = 0x%x\n",
					  sat_tab->fix_beam_pattern_codeword);
			else
				phydm_fast_ant_training_hl_smart_antenna_type2(dm);
		}
#endif
	}
#endif


}


void odm_antsel_statistics(
	void			*dm_void,
	void			*phy_info_void,
	u8			antsel_tr_mux,
	u32			mac_id,
	u32			utility,
	u8			method,
	u8			is_cck_rate

)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;
	struct phydm_phyinfo_struct	*phy_info = (struct phydm_phyinfo_struct *)
			phy_info_void;

	if (method == RSSI_METHOD) {
		if (is_cck_rate) {
			if (antsel_tr_mux == ANT1_2G) {
				if (fat_tab->main_ant_sum_cck[mac_id] >
				    65435) /*to prevent u16 overflow, max(RSSI)=100, 65435+100 = 65535 (u16)*/
					return;

				fat_tab->main_ant_sum_cck[mac_id] += (u16)utility;
				fat_tab->main_ant_cnt_cck[mac_id]++;
			} else {
				if (fat_tab->aux_ant_sum_cck[mac_id] > 65435)
					return;

				fat_tab->aux_ant_sum_cck[mac_id] += (u16)utility;
				fat_tab->aux_ant_cnt_cck[mac_id]++;
			}

		} else { /*ofdm rate*/

			if (antsel_tr_mux == ANT1_2G) {
				if (fat_tab->main_ant_sum[mac_id] > 65435)
					return;

				fat_tab->main_ant_sum[mac_id] += (u16)utility;
				fat_tab->main_ant_cnt[mac_id]++;
			} else {
				if (fat_tab->aux_ant_sum[mac_id] > 65435)
					return;

				fat_tab->aux_ant_sum[mac_id] += (u16)utility;
				fat_tab->aux_ant_cnt[mac_id]++;
			}
		}
	}
#ifdef ODM_EVM_ENHANCE_ANTDIV
	else if (method == EVM_METHOD) {
		if (dm->rate_ss == 1) {
			if (antsel_tr_mux == ANT1_2G) {
				fat_tab->main_ant_evm_sum[mac_id] += ((phy_info->rx_mimo_evm_dbm[0]) << 5);
				fat_tab->main_ant_evm_cnt[mac_id]++;
			} else {
				fat_tab->aux_ant_evm_sum[mac_id] += ((phy_info->rx_mimo_evm_dbm[0]) << 5);
				fat_tab->aux_ant_evm_cnt[mac_id]++;
			}

		} else {/*>= 2SS*/

			if (antsel_tr_mux == ANT1_2G) {
				fat_tab->main_ant_evm_2ss_sum[mac_id][0] += (phy_info->rx_mimo_evm_dbm[0]
						<< 5);
				fat_tab->main_ant_evm_2ss_sum[mac_id][1] += (phy_info->rx_mimo_evm_dbm[1]
						<< 5);
				fat_tab->main_ant_evm_2ss_cnt[mac_id]++;

			} else {
				fat_tab->aux_ant_evm_2ss_sum[mac_id][0] += (phy_info->rx_mimo_evm_dbm[0] <<
						5);
				fat_tab->aux_ant_evm_2ss_sum[mac_id][1] += (phy_info->rx_mimo_evm_dbm[1] <<
						5);
				fat_tab->aux_ant_evm_2ss_cnt[mac_id]++;
			}
		}

	} else if (method == CRC32_METHOD) {
		if (antsel_tr_mux == ANT1_2G) {
			fat_tab->main_crc32_ok_cnt += utility;
			fat_tab->main_crc32_fail_cnt++;
		} else {
			fat_tab->aux_crc32_ok_cnt += utility;
			fat_tab->aux_crc32_fail_cnt++;
		}

	} else if (method == TP_METHOD) {
		if (((utility <= ODM_RATEMCS15) && (utility >= ODM_RATEMCS0)) &&
		    (fat_tab->fat_state_cnt <= dm->antdiv_tp_period)
		   ) {
			if (antsel_tr_mux == ANT1_2G) {
				fat_tab->antdiv_tp_main += (phy_rate_table[utility]) << 5;
				fat_tab->antdiv_tp_main_cnt++;
			} else {
				fat_tab->antdiv_tp_aux += (phy_rate_table[utility]) << 5;
				fat_tab->antdiv_tp_aux_cnt++;
			}
		}
	}
#endif
}

void odm_process_rssi_for_ant_div(
	void			*dm_void,
	void			*phy_info_void,
	void			*pkt_info_void
)
{
	struct dm_struct				*dm = (struct dm_struct *)dm_void;
	struct phydm_phyinfo_struct			*phy_info = (struct phydm_phyinfo_struct *)
			phy_info_void;
	struct phydm_perpkt_info_struct				*pktinfo = (struct
			phydm_perpkt_info_struct *)pkt_info_void;
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;
#if (defined(CONFIG_HL_SMART_ANTENNA))
	struct smt_ant_honbo	*sat_tab = &dm->dm_sat_table;
	u32			beam_tmp;
	u8			next_ant;
	u8			train_pkt_number;
#endif
	u8			rx_power_ant0 = phy_info->rx_mimo_signal_strength[0];
	u8			rx_power_ant1 = phy_info->rx_mimo_signal_strength[1];
	u8			rx_evm_ant0 = phy_info->rx_mimo_signal_quality[0];
	u8			rx_evm_ant1 = phy_info->rx_mimo_signal_quality[1];
	u8			rssi_avg;

	if ((dm->support_ic_type & ODM_IC_2SS) && (!pktinfo->is_cck_rate)) {
		if (rx_power_ant1 < 100)
			rssi_avg = (u8)odm_convert_to_db((odm_convert_to_linear(
					rx_power_ant0) + odm_convert_to_linear(rx_power_ant1)) >>
							 1); /*averaged PWDB*/

	} else {
		rx_power_ant0 = (u8)phy_info->rx_pwdb_all;
		rssi_avg = rx_power_ant0;
	}

#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2
	if ((dm->ant_div_type == HL_SW_SMART_ANT_TYPE2)
	    && (fat_tab->fat_state == FAT_TRAINING_STATE))
		phydm_process_rssi_for_hb_smtant_type2(dm, phy_info, pktinfo,
						       rssi_avg);	/*for 8822B*/
	else
#endif

#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
#ifdef CONFIG_FAT_PATCH
		if ((dm->ant_div_type == HL_SW_SMART_ANT_TYPE1)
		    && (fat_tab->fat_state == FAT_TRAINING_STATE)) {
			/*[Beacon]*/
			if (pktinfo->is_packet_beacon) {
				sat_tab->beacon_counter++;
				PHYDM_DBG(dm, DBG_ANT_DIV, "MatchBSSID_beacon_counter = ((%d))\n",
					  sat_tab->beacon_counter);

				if (sat_tab->beacon_counter >= sat_tab->pre_beacon_counter + 2) {
					if (sat_tab->ant_num > 1) {
						next_ant = (fat_tab->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
						odm_update_rx_idle_ant(dm, next_ant);
					}

					sat_tab->update_beam_idx++;

					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "pre_beacon_counter = ((%d)), pkt_counter = ((%d)), update_beam_idx = ((%d))\n",
						  sat_tab->pre_beacon_counter, sat_tab->pkt_counter,
						  sat_tab->update_beam_idx);

					sat_tab->pre_beacon_counter = sat_tab->beacon_counter;
					sat_tab->pkt_counter = 0;
				}
			}
			/*[data]*/
			else if (pktinfo->is_packet_to_self) {
				if (sat_tab->pkt_skip_statistic_en == 0) {
					/*
					PHYDM_DBG(dm, DBG_ANT_DIV, "StaID[%d]:  antsel_pathA = ((%d)), hw_antsw_occur = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
						pktinfo->station_id, fat_tab->antsel_rx_keep_0, fat_tab->hw_antsw_occur, sat_tab->fast_training_beam_num, rx_power_ant0);
					*/
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "ID[%d][pkt_cnt = %d]: {ANT, Beam} = {%d, %d}, RSSI = ((%d))\n",
						  pktinfo->station_id, sat_tab->pkt_counter, fat_tab->antsel_rx_keep_0,
						  sat_tab->fast_training_beam_num, rx_power_ant0);

					sat_tab->pkt_rssi_sum[fat_tab->antsel_rx_keep_0][sat_tab->fast_training_beam_num]
					+= rx_power_ant0;
					sat_tab->pkt_rssi_cnt[fat_tab->antsel_rx_keep_0][sat_tab->fast_training_beam_num]++;
					sat_tab->pkt_counter++;

#if 1
					train_pkt_number = sat_tab->beam_train_cnt[fat_tab->rx_idle_ant -
							   1][sat_tab->fast_training_beam_num];
#else
					train_pkt_number =  sat_tab->per_beam_training_pkt_num;
#endif

					/*Swich Antenna erery N pkts*/
					if (sat_tab->pkt_counter == train_pkt_number) {
						if (sat_tab->ant_num > 1) {
							PHYDM_DBG(dm, DBG_ANT_DIV,
								  "packet enugh ((%d ))pkts ---> Switch antenna\n", train_pkt_number);
							next_ant = (fat_tab->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
							odm_update_rx_idle_ant(dm, next_ant);
						}

						sat_tab->update_beam_idx++;
						PHYDM_DBG(dm, DBG_ANT_DIV,
							  "pre_beacon_counter = ((%d)), update_beam_idx_counter = ((%d))\n",
							  sat_tab->pre_beacon_counter, sat_tab->update_beam_idx);

						sat_tab->pre_beacon_counter = sat_tab->beacon_counter;
						sat_tab->pkt_counter = 0;
					}
				}
			}

			/*Swich Beam after switch "sat_tab->ant_num" antennas*/
			if (sat_tab->update_beam_idx == sat_tab->ant_num) {
				sat_tab->update_beam_idx = 0;
				sat_tab->pkt_counter = 0;
				beam_tmp = sat_tab->fast_training_beam_num;

				if (sat_tab->fast_training_beam_num >= (sat_tab->beam_patten_num_each_ant -
									1)) {
					fat_tab->fat_state = FAT_DECISION_STATE;

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
					odm_fast_ant_training_hl_smart_antenna_type1(dm);
#else
					odm_schedule_work_item(&sat_tab->hl_smart_antenna_decision_workitem);
#endif


				} else {
					sat_tab->fast_training_beam_num++;
					PHYDM_DBG(dm, DBG_ANT_DIV, "Update Beam_num (( %d )) -> (( %d ))\n",
						  beam_tmp, sat_tab->fast_training_beam_num);
					phydm_set_all_ant_same_beam_num(dm);

					fat_tab->fat_state = FAT_TRAINING_STATE;
				}
			}

		}
#else

		if (dm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {
			if ((dm->support_ic_type & ODM_HL_SMART_ANT_TYPE1_SUPPORT) &&
			    (pktinfo->is_packet_to_self)   &&
			    (fat_tab->fat_state == FAT_TRAINING_STATE)
			   ) {
				if (sat_tab->pkt_skip_statistic_en == 0) {
					/*
					PHYDM_DBG(dm, DBG_ANT_DIV, "StaID[%d]:  antsel_pathA = ((%d)), hw_antsw_occur = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
						pktinfo->station_id, fat_tab->antsel_rx_keep_0, fat_tab->hw_antsw_occur, sat_tab->fast_training_beam_num, rx_power_ant0);
					*/
					PHYDM_DBG(dm, DBG_ANT_DIV,
						  "StaID[%d]:  antsel_pathA = ((%d)), is_packet_to_self = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
						  pktinfo->station_id, fat_tab->antsel_rx_keep_0, pktinfo->is_packet_to_self,
						  sat_tab->fast_training_beam_num, rx_power_ant0);


					sat_tab->pkt_rssi_sum[fat_tab->antsel_rx_keep_0][sat_tab->fast_training_beam_num]
					+= rx_power_ant0;
					sat_tab->pkt_rssi_cnt[fat_tab->antsel_rx_keep_0][sat_tab->fast_training_beam_num]++;
					sat_tab->pkt_counter++;

					/*swich beam every N pkt*/
					if ((sat_tab->pkt_counter) >= (sat_tab->per_beam_training_pkt_num)) {
						sat_tab->pkt_counter = 0;
						beam_tmp = sat_tab->fast_training_beam_num;

						if (sat_tab->fast_training_beam_num >= (sat_tab->beam_patten_num_each_ant -
											1)) {
							fat_tab->fat_state = FAT_DECISION_STATE;

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
							odm_fast_ant_training_hl_smart_antenna_type1(dm);
#else
							odm_schedule_work_item(&sat_tab->hl_smart_antenna_decision_workitem);
#endif


						} else {
							sat_tab->fast_training_beam_num++;
							phydm_set_all_ant_same_beam_num(dm);

							fat_tab->fat_state = FAT_TRAINING_STATE;
							PHYDM_DBG(dm, DBG_ANT_DIV, "Update  Beam_num (( %d )) -> (( %d ))\n",
								  beam_tmp, sat_tab->fast_training_beam_num);
						}
					}
				}
			}
		}
#endif
		else
#endif
			if (dm->ant_div_type == CG_TRX_SMART_ANTDIV) {
				if ((dm->support_ic_type & ODM_SMART_ANT_SUPPORT)
				    && (pktinfo->is_packet_to_self)
				    && (fat_tab->fat_state ==
					FAT_TRAINING_STATE)) { /* (pktinfo->is_packet_match_bssid && (!pktinfo->is_packet_beacon)) */
					u8	antsel_tr_mux;
					antsel_tr_mux = (fat_tab->antsel_rx_keep_2 << 2) |
							(fat_tab->antsel_rx_keep_1 << 1) | fat_tab->antsel_rx_keep_0;
					fat_tab->ant_sum_rssi[antsel_tr_mux] += rx_power_ant0;
					fat_tab->ant_rssi_cnt[antsel_tr_mux]++;
				}
			} else { /* ant_div_type != CG_TRX_SMART_ANTDIV */
				if ((dm->support_ic_type & ODM_ANTDIV_SUPPORT)
				    && (pktinfo->is_packet_to_self || fat_tab->use_ctrl_frame_antdiv)) {
					if (dm->ant_div_type == S0S1_SW_ANTDIV) {
						if (pktinfo->is_cck_rate || (dm->support_ic_type == ODM_RTL8188F))
							fat_tab->antsel_rx_keep_0 = (fat_tab->rx_idle_ant == MAIN_ANT) ? ANT1_2G :
										    ANT2_2G;

						odm_antsel_statistics(dm, phy_info, fat_tab->antsel_rx_keep_0,
								      pktinfo->station_id, rx_power_ant0, RSSI_METHOD, pktinfo->is_cck_rate);

					} else {

						odm_antsel_statistics(dm, phy_info, fat_tab->antsel_rx_keep_0,
								      pktinfo->station_id, rx_power_ant0, RSSI_METHOD, pktinfo->is_cck_rate);

#ifdef ODM_EVM_ENHANCE_ANTDIV
						if (dm->support_ic_type == ODM_RTL8192E) {
							if (!pktinfo->is_cck_rate) {
								odm_antsel_statistics(dm, phy_info, fat_tab->antsel_rx_keep_0,
										      pktinfo->station_id, rx_evm_ant0, EVM_METHOD, pktinfo->is_cck_rate);
								odm_antsel_statistics(dm, phy_info, fat_tab->antsel_rx_keep_0,
										      pktinfo->station_id, rx_evm_ant0, TP_METHOD, pktinfo->is_cck_rate);
							}

						}
#endif
					}
				}
			}
	/* PHYDM_DBG(dm,DBG_ANT_DIV,"is_cck_rate=%d, pwdb_all=%d\n",pktinfo->is_cck_rate, phy_info->rx_pwdb_all); */
	/* PHYDM_DBG(dm,DBG_ANT_DIV,"antsel_tr_mux=3'b%d%d%d\n",fat_tab->antsel_rx_keep_2, fat_tab->antsel_rx_keep_1, fat_tab->antsel_rx_keep_0); */
}

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
void odm_set_tx_ant_by_tx_info(
	void			*dm_void,
	u8			*desc,
	u8			mac_id

)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct	*fat_tab = &dm->dm_fat_table;

	if (!(dm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (dm->ant_div_type == CGCS_RX_HW_ANTDIV)
		return;


	if (dm->support_ic_type == ODM_RTL8723B) {
#if (RTL8723B_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8723B(desc, fat_tab->antsel_a[mac_id]);
		/*PHYDM_DBG(dm,DBG_ANT_DIV, "[8723B] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, fat_tab->antsel_c[mac_id], fat_tab->antsel_b[mac_id], fat_tab->antsel_a[mac_id]);*/
#endif
	} else if (dm->support_ic_type == ODM_RTL8821) {
#if (RTL8821A_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8812(desc, fat_tab->antsel_a[mac_id]);
		/*PHYDM_DBG(dm,DBG_ANT_DIV, "[8821A] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, fat_tab->antsel_c[mac_id], fat_tab->antsel_b[mac_id], fat_tab->antsel_a[mac_id]);*/
#endif
	} else if (dm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_88E(desc, fat_tab->antsel_a[mac_id]);
		SET_TX_DESC_ANTSEL_B_88E(desc, fat_tab->antsel_b[mac_id]);
		SET_TX_DESC_ANTSEL_C_88E(desc, fat_tab->antsel_c[mac_id]);
		/*PHYDM_DBG(dm,DBG_ANT_DIV, "[8188E] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, fat_tab->antsel_c[mac_id], fat_tab->antsel_b[mac_id], fat_tab->antsel_a[mac_id]);*/
#endif
	} else if (dm->support_ic_type == ODM_RTL8821C) {
#if (RTL8821C_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8821C(desc, fat_tab->antsel_a[mac_id]);
		/*PHYDM_DBG(dm,DBG_ANT_DIV, "[8821C] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, fat_tab->antsel_c[mac_id], fat_tab->antsel_b[mac_id], fat_tab->antsel_a[mac_id]);*/
#endif
	}
}
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)

void odm_set_tx_ant_by_tx_info(
	struct	rtl8192cd_priv		*priv,
	struct	tx_desc	*pdesc,
	unsigned short			aid
)
{
	struct dm_struct	*dm = GET_PDM_ODM(priv);/*&(priv->pshare->_dmODM);*/
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;

	if (!(dm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (dm->ant_div_type == CGCS_RX_HW_ANTDIV)
		return;

	if (dm->support_ic_type == ODM_RTL8881A) {
		/*panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8881E******\n",__FUNCTION__,__LINE__);	*/
		pdesc->Dword6 &= set_desc(~(BIT(18) | BIT(17) | BIT(16)));
		pdesc->Dword6 |= set_desc(fat_tab->antsel_a[aid] << 16);
	} else if (dm->support_ic_type == ODM_RTL8192E) {
		/*panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8192E******\n",__FUNCTION__,__LINE__);	*/
		pdesc->Dword6 &= set_desc(~(BIT(18) | BIT(17) | BIT(16)));
		pdesc->Dword6 |= set_desc(fat_tab->antsel_a[aid] << 16);
	} else if (dm->support_ic_type == ODM_RTL8188E) {
		/*panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8188E******\n",__FUNCTION__,__LINE__);*/
		pdesc->Dword2 &= set_desc(~BIT(24));
		pdesc->Dword2 &= set_desc(~BIT(25));
		pdesc->Dword7 &= set_desc(~BIT(29));

		pdesc->Dword2 |= set_desc(fat_tab->antsel_a[aid] << 24);
		pdesc->Dword2 |= set_desc(fat_tab->antsel_b[aid] << 25);
		pdesc->Dword7 |= set_desc(fat_tab->antsel_c[aid] << 29);


	} else if (dm->support_ic_type == ODM_RTL8812) {
		/*[path-A]*/
		/*panic_printk("[%s] [%d]   ******ODM_SetTxAntByTxInfo_8881E******\n",__FUNCTION__,__LINE__);*/

		pdesc->Dword6 &= set_desc(~BIT(16));
		pdesc->Dword6 &= set_desc(~BIT(17));
		pdesc->Dword6 &= set_desc(~BIT(18));

		pdesc->Dword6 |= set_desc(fat_tab->antsel_a[aid] << 16);
		pdesc->Dword6 |= set_desc(fat_tab->antsel_b[aid] << 17);
		pdesc->Dword6 |= set_desc(fat_tab->antsel_c[aid] << 18);

	}
}


#if 1 /*def CONFIG_WLAN_HAL*/
void odm_set_tx_ant_by_tx_info_hal(
	struct	rtl8192cd_priv		*priv,
	void	*pdesc_data,
	u16					aid
)
{
	struct dm_struct	*dm = GET_PDM_ODM(priv);/*&(priv->pshare->_dmODM);*/
	struct phydm_fat_struct		*fat_tab = &dm->dm_fat_table;
	PTX_DESC_DATA_88XX	pdescdata = (PTX_DESC_DATA_88XX)pdesc_data;

	if (!(dm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (dm->ant_div_type == CGCS_RX_HW_ANTDIV)
		return;

	if (dm->support_ic_type & (ODM_RTL8881A | ODM_RTL8192E | ODM_RTL8814A)) {
		/*panic_printk("[%s] [%d] ******odm_set_tx_ant_by_tx_info_hal******\n",__FUNCTION__,__LINE__);*/
		pdescdata->ant_sel = 1;
		pdescdata->ant_sel_a = fat_tab->antsel_a[aid];
	}
}
#endif	/*#ifdef CONFIG_WLAN_HAL*/

#endif


void odm_ant_div_config(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	struct phydm_fat_struct			*fat_tab = &dm->dm_fat_table;
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	PHYDM_DBG(dm, DBG_ANT_DIV, "WIN Config Antenna Diversity\n");
	/*
	if(dm->support_ic_type==ODM_RTL8723B)
	{
		if((!dm->dm_swat_table.ANTA_ON || !dm->dm_swat_table.ANTB_ON))
			dm->support_ability &= ~(ODM_BB_ANT_DIV);
	}
	*/
	if (dm->support_ic_type == ODM_RTL8723D) {
		dm->ant_div_type = S0S1_TRX_HW_ANTDIV;
		/**/
	}
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE))

	PHYDM_DBG(dm, DBG_ANT_DIV, "CE Config Antenna Diversity\n");

	if (dm->support_ic_type == ODM_RTL8723B)
		dm->ant_div_type = S0S1_SW_ANTDIV;



#elif (DM_ODM_SUPPORT_TYPE & (ODM_AP))

	PHYDM_DBG(dm, DBG_ANT_DIV, "AP Config Antenna Diversity\n");

	/* 2 [ NOT_SUPPORT_ANTDIV ] */
#if (defined(CONFIG_NOT_SUPPORT_ANTDIV))
	dm->support_ability &= ~(ODM_BB_ANT_DIV);
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Disable AntDiv function] : Not Support 2.4G & 5G Antenna Diversity\n");

	/* 2 [ 2G&5G_SUPPORT_ANTDIV ] */
#elif (defined(CONFIG_2G5G_SUPPORT_ANTDIV))
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Enable AntDiv function] : 2.4G & 5G Support Antenna Diversity Simultaneously\n");
	fat_tab->ant_div_2g_5g = (ODM_ANTDIV_2G | ODM_ANTDIV_5G);

	if (dm->support_ic_type & ODM_ANTDIV_SUPPORT)
		dm->support_ability |= ODM_BB_ANT_DIV;
	if (*dm->band_type == ODM_BAND_5G) {
#if (defined(CONFIG_5G_CGCS_RX_DIVERSITY))
		dm->ant_div_type = CGCS_RX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
		panic_printk("[ 5G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
#elif (defined(CONFIG_5G_CG_TRX_DIVERSITY) || defined(CONFIG_2G5G_CG_TRX_DIVERSITY_8881A))
		dm->ant_div_type = CG_TRX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
		panic_printk("[ 5G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY))
		dm->ant_div_type = CG_TRX_SMART_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CG_SMART_ANTDIV\n");
#elif (defined(CONFIG_5G_S0S1_SW_ANT_DIVERSITY))
		dm->ant_div_type = S0S1_SW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = S0S1_SW_ANTDIV\n");
#endif
	} else if (*dm->band_type == ODM_BAND_2_4G) {
#if (defined(CONFIG_2G_CGCS_RX_DIVERSITY))
		dm->ant_div_type = CGCS_RX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
#elif (defined(CONFIG_2G_CG_TRX_DIVERSITY) || defined(CONFIG_2G5G_CG_TRX_DIVERSITY_8881A))
		dm->ant_div_type = CG_TRX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
#elif (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		dm->ant_div_type = CG_TRX_SMART_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CG_SMART_ANTDIV\n");
#elif (defined(CONFIG_2G_S0S1_SW_ANT_DIVERSITY))
		dm->ant_div_type = S0S1_SW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = S0S1_SW_ANTDIV\n");
#endif
	}

	/* 2 [ 5G_SUPPORT_ANTDIV ] */
#elif (defined(CONFIG_5G_SUPPORT_ANTDIV))
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Enable AntDiv function] : Only 5G Support Antenna Diversity\n");
	panic_printk("[ Enable AntDiv function] : Only 5G Support Antenna Diversity\n");
	fat_tab->ant_div_2g_5g = (ODM_ANTDIV_5G);
	if (*dm->band_type == ODM_BAND_5G) {
		if (dm->support_ic_type & ODM_ANTDIV_5G_SUPPORT_IC)
			dm->support_ability |= ODM_BB_ANT_DIV;
#if (defined(CONFIG_5G_CGCS_RX_DIVERSITY))
		dm->ant_div_type = CGCS_RX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
		panic_printk("[ 5G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
#elif (defined(CONFIG_5G_CG_TRX_DIVERSITY))
		dm->ant_div_type = CG_TRX_HW_ANTDIV;
		panic_printk("[ 5G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY))
		dm->ant_div_type = CG_TRX_SMART_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = CG_SMART_ANTDIV\n");
#elif (defined(CONFIG_5G_S0S1_SW_ANT_DIVERSITY))
		dm->ant_div_type = S0S1_SW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 5G] : AntDiv type = S0S1_SW_ANTDIV\n");
#endif
	} else if (*dm->band_type == ODM_BAND_2_4G) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "Not Support 2G ant_div_type\n");
		dm->support_ability &= ~(ODM_BB_ANT_DIV);
	}

	/* 2 [ 2G_SUPPORT_ANTDIV ] */
#elif (defined(CONFIG_2G_SUPPORT_ANTDIV))
	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[ Enable AntDiv function] : Only 2.4G Support Antenna Diversity\n");
	fat_tab->ant_div_2g_5g = (ODM_ANTDIV_2G);
	if (*dm->band_type == ODM_BAND_2_4G) {
		if (dm->support_ic_type & ODM_ANTDIV_2G_SUPPORT_IC)
			dm->support_ability |= ODM_BB_ANT_DIV;
#if (defined(CONFIG_2G_CGCS_RX_DIVERSITY))
		dm->ant_div_type = CGCS_RX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CGCS_RX_HW_ANTDIV\n");
#elif (defined(CONFIG_2G_CG_TRX_DIVERSITY))
		dm->ant_div_type = CG_TRX_HW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CG_TRX_HW_ANTDIV\n");
#elif (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		dm->ant_div_type = CG_TRX_SMART_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = CG_SMART_ANTDIV\n");
#elif (defined(CONFIG_2G_S0S1_SW_ANT_DIVERSITY))
		dm->ant_div_type = S0S1_SW_ANTDIV;
		PHYDM_DBG(dm, DBG_ANT_DIV, "[ 2.4G] : AntDiv type = S0S1_SW_ANTDIV\n");
#endif
	} else if (*dm->band_type == ODM_BAND_5G) {
		PHYDM_DBG(dm, DBG_ANT_DIV, "Not Support 5G ant_div_type\n");
		dm->support_ability &= ~(ODM_BB_ANT_DIV);
	}
#endif
#endif

	PHYDM_DBG(dm, DBG_ANT_DIV,
		  "[AntDiv Config Info] AntDiv_SupportAbility = (( %x ))\n",
		  ((dm->support_ability & ODM_BB_ANT_DIV) ? 1 : 0));
	PHYDM_DBG(dm, DBG_ANT_DIV, "[AntDiv Config Info] be_fix_tx_ant = ((%d))\n",
		  dm->dm_fat_table.b_fix_tx_ant);

}


void odm_ant_div_timers(
	void		*dm_void,
	u8		state
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	if (state == INIT_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_initialize_timer(dm,
				     &dm->dm_swat_table.phydm_sw_antenna_switch_timer,
				     (void *)odm_sw_antdiv_callback, NULL,
				     "phydm_sw_antenna_switch_timer");
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		odm_initialize_timer(dm, &dm->fast_ant_training_timer,
				     (void *)odm_fast_ant_training_callback, NULL, "fast_ant_training_timer");
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
		odm_initialize_timer(dm, &dm->evm_fast_ant_training_timer,
				     (void *)odm_evm_fast_ant_training_callback, NULL,
				     "evm_fast_ant_training_timer");
#endif
	} else if (state == CANCEL_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_cancel_timer(dm,
				 &dm->dm_swat_table.phydm_sw_antenna_switch_timer);
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		odm_cancel_timer(dm, &dm->fast_ant_training_timer);
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
		odm_cancel_timer(dm, &dm->evm_fast_ant_training_timer);
#endif
	} else if (state == RELEASE_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_release_timer(dm,
				  &dm->dm_swat_table.phydm_sw_antenna_switch_timer);
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		odm_release_timer(dm, &dm->fast_ant_training_timer);
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
		odm_release_timer(dm, &dm->evm_fast_ant_training_timer);
#endif
	}

}

void phydm_antdiv_debug(
	void		*dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
	/*struct phydm_fat_struct*			fat_tab = &dm->dm_fat_table;*/
	u32 used = *_used;
	u32 out_len = *_out_len;

	if (dm_value[0] == 1) { /*fixed or auto antenna*/

		if (dm_value[1] == 0) {
			dm->ant_type = ODM_AUTO_ANT;
			PDM_SNPF(out_len, used, output + used,
				 out_len - used, "AntDiv: Auto\n");
		} else if (dm_value[1] == 1) {
			dm->ant_type = ODM_FIX_MAIN_ANT;
			PDM_SNPF(out_len, used, output + used,
				 out_len - used, "AntDiv: Fix Main\n");
		} else if (dm_value[1] == 2) {
			dm->ant_type = ODM_FIX_AUX_ANT;
			PDM_SNPF(out_len, used, output + used,
				 out_len - used, "AntDiv: Fix Aux\n");
		}

		if (dm->ant_type != ODM_AUTO_ANT) {
			odm_stop_antenna_switch_dm(dm);
			if (dm->ant_type == ODM_FIX_MAIN_ANT)
				odm_update_rx_idle_ant(dm, MAIN_ANT);
			else if (dm->ant_type == ODM_FIX_AUX_ANT)
				odm_update_rx_idle_ant(dm, AUX_ANT);
		} else {
			phydm_enable_antenna_diversity(dm);
		}
		dm->pre_ant_type = dm->ant_type;
	} else if (dm_value[0] == 2) { /*dynamic period for AntDiv*/

		dm->antdiv_period = (u8)dm_value[1];
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "AntDiv_period = ((%d))\n", dm->antdiv_period);
	}
	*_used = used;
	*_out_len = out_len;
}

#endif /*#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))*/

void odm_ant_div_reset(
	void		*dm_void
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	if (dm->ant_div_type == S0S1_SW_ANTDIV) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_s0s1_sw_ant_div_reset(dm);
#endif
	}

}

void odm_antenna_diversity_init(
	void		*dm_void
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	struct dm_struct		*dm = (struct dm_struct *)dm_void;

#if 0
	if (*(dm->mp_mode) == true)
		return;
#endif

	odm_ant_div_config(dm);
	odm_ant_div_init(dm);
#endif
}

void odm_antenna_diversity(
	void		*dm_void
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

	struct dm_struct		*dm = (struct dm_struct *)dm_void;

	if (*dm->mp_mode == true)
		return;

	if (!(dm->support_ability & ODM_BB_ANT_DIV)) {
		PHYDM_DBG(dm, DBG_ANT_DIV,
			  "[Return!!!]   Not Support Antenna Diversity Function\n");
		return;
	}

	if (dm->pause_ability & ODM_BB_ANT_DIV) {

		PHYDM_DBG(dm, DBG_ANT_DIV, "Return: Pause AntDIv in LV=%d\n",
			  dm->pause_lv_table.lv_antdiv);
		return;
	}

	odm_ant_div(dm);
#endif
}
