/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
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
#ifndef __RTL8192E_RF_H__
#define __RTL8192E_RF_H__

VOID PHY_RF6052SetBandwidth8192E(
	IN	PADAPTER				Adapter,
	IN	CHANNEL_WIDTH		Bandwidth);


int PHY_RF6052_Config_8192E(
	IN	PADAPTER	Adapter);

VOID PHY_RF6052SetCckTxPower8192E(
	IN	PADAPTER	Adapter,
	IN	u8			*pPowerlevel);

VOID PHY_RF6052SetOFDMTxPower8192E(
	IN	PADAPTER	Adapter,
	IN	u8			*pPowerLevelOFDM,
	IN	u8			*pPowerLevelBW20,
	IN	u8			*pPowerLevelBW40,
	IN	u8			Channel);

#endif//__RTL8192E_RF_H__

