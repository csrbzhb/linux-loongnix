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

#ifndef	__PHYDMDYNAMICTXPOWER_H__
#define    __PHYDMDYNAMICTXPOWER_H__

/*#define DYNAMIC_TXPWR_VERSION	"1.0"*/
/*#define DYNAMIC_TXPWR_VERSION	"1.3" */ /*2015.08.26, Add 8814 Dynamic TX power*/
#define DYNAMIC_TXPWR_VERSION	"1.4" /*2015.11.06, Add CE 8821A Dynamic TX power*/

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	60
#define		TX_POWER_NEAR_FIELD_THRESH_AP	0x3F
#elif (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	60
#endif

#define		tx_high_pwr_level_normal		0
#define		tx_high_pwr_level_level1		1
#define		tx_high_pwr_level_level2		2

#define		tx_high_pwr_level_bt1			3
#define		tx_high_pwr_level_bt2			4
#define		tx_high_pwr_level_15			5
#define		tx_high_pwr_level_35			6
#define		tx_high_pwr_level_50			7
#define		tx_high_pwr_level_70			8
#define		tx_high_pwr_level_100			9

enum phydm_dtp_power_offset {
	PHYDM_OFFSET_ZERO = 0,
	PHYDM_OFFSET_MINUS_3DB = 1,
	PHYDM_OFFSET_MINUS_7DB = 2,
	PHYDM_OFFSET_MINUS_11DB = 3,
	PHYDM_OFFSET_ADD_3DB = 4,
	PHYDM_OFFSET_ADD_6DB = 5
};

void phydm_pow_train_init(
	void					*dm_void
);

void phydm_dynamic_tx_power(
	void					*dm_void
);

void odm_dynamic_tx_power_restore_power_index(
	void					*dm_void
);

void odm_dynamic_tx_power_nic(
	void					*dm_void
);

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
void odm_dynamic_tx_power_save_power_index(
	void					*dm_void
);

void odm_dynamic_tx_power_write_power_index(
	void					*dm_void,
	u8		value);

void odm_dynamic_tx_power_8821(
	void					*dm_void,
	u8					*desc,
	u8					mac_id
);
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
void odm_dynamic_tx_power_8814a(
	void					*dm_void
);


void odm_set_tx_power_level8814(
	void		*adapter,
	u8			channel,
	u8			pwr_lvl
);
#endif
#endif

void odm_dynamic_tx_power(
	void					*dm_void
);

void phydm_dynamic_tx_power(
	void					*dm_void
);

void phydm_dynamic_tx_power_init(
	void					*dm_void
);

#endif
