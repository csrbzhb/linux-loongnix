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
#ifndef __RTL8188E_HAL_H__
#define __RTL8188E_HAL_H__

//#include "hal_com.h"
#if 1
#include "hal_data.h"
#else
#include "../hal/OUTSRC/odm_precomp.h"
#endif

//include HAL Related header after HAL Related compiling flags
#include "rtl8188e_spec.h"
#include "Hal8188EPhyReg.h"
#include "Hal8188EPhyCfg.h"
#include "rtl8188e_rf.h"
#include "rtl8188e_dm.h"
#include "rtl8188e_recv.h"
#include "rtl8188e_xmit.h"
#include "rtl8188e_cmd.h"
#include "rtl8188e_led.h"
#include "Hal8188EPwrSeq.h"
#ifdef DBG_CONFIG_ERROR_DETECT
#include "rtl8188e_sreset.h"
#endif


// Fw Array
#define Rtl8188E_FwImageArray				Rtl8188EFwImgArray
#define Rtl8188E_FWImgArrayLength			Rtl8188EFWImgArrayLength
#ifdef CONFIG_WOWLAN
#define Rtl8188E_FwWoWImageArray			Array_MP_8188E_FW_WoWLAN
#define Rtl8188E_FwWoWImgArrayLength		ArrayLength_MP_8188E_FW_WoWLAN
#endif //CONFIG_WOWLAN


#ifdef CONFIG_SDIO_HCI

//TODO: We should define 8188ES firmware related macro settings here!!
//TODO:  The following need to check!!
#define RTL8188E_FW_UMC_IMG				"rtl8188E\\rtl8188efw.bin"
#define RTL8188E_PHY_REG					"rtl8188E\\PHY_REG_1T.txt"
#define RTL8188E_PHY_RADIO_A				"rtl8188E\\radio_a_1T.txt"
#define RTL8188E_PHY_RADIO_B				"rtl8188E\\radio_b_1T.txt"
#define RTL8188E_AGC_TAB					"rtl8188E\\AGC_TAB_1T.txt"
#define RTL8188E_PHY_MACREG 				"rtl8188E\\MAC_REG.txt"
#define RTL8188E_PHY_REG_PG				"rtl8188E\\PHY_REG_PG.txt"
#define RTL8188E_PHY_REG_MP				"rtl8188E\\PHY_REG_MP.txt"

//---------------------------------------------------------------------
//		RTL8188E From header
//---------------------------------------------------------------------
#if 0
#define Rtl8188E_PHY_REG_Array_PG			Rtl8188ESPHY_REG_Array_PG
#define Rtl8188E_PHY_REG_Array_PGLength	Rtl8188ESPHY_REG_Array_PGLength

#endif

//---------------------------------------------------------------------
//		RTL8188E Power Configuration CMDs for USB/SDIO interfaces
//---------------------------------------------------------------------
#define Rtl8188E_NIC_PWR_ON_FLOW				rtl8188E_power_on_flow
#define Rtl8188E_NIC_RF_OFF_FLOW				rtl8188E_radio_off_flow
#define Rtl8188E_NIC_DISABLE_FLOW				rtl8188E_card_disable_flow
#define Rtl8188E_NIC_ENABLE_FLOW				rtl8188E_card_enable_flow
#define Rtl8188E_NIC_SUSPEND_FLOW				rtl8188E_suspend_flow
#define Rtl8188E_NIC_RESUME_FLOW				rtl8188E_resume_flow
#define Rtl8188E_NIC_PDN_FLOW					rtl8188E_hwpdn_flow
#define Rtl8188E_NIC_LPS_ENTER_FLOW			rtl8188E_enter_lps_flow
#define Rtl8188E_NIC_LPS_LEAVE_FLOW			rtl8188E_leave_lps_flow

#elif defined(CONFIG_USB_HCI)
#define RTL8188E_FW_UMC_IMG				"rtl8188E\\rtl8188efw.bin"
#define RTL8188E_PHY_REG					"rtl8188E\\PHY_REG_1T.txt"
#define RTL8188E_PHY_RADIO_A				"rtl8188E\\radio_a_1T.txt"
#define RTL8188E_PHY_RADIO_B				"rtl8188E\\radio_b_1T.txt"
#define RTL8188E_AGC_TAB					"rtl8188E\\AGC_TAB_1T.txt"
#define RTL8188E_PHY_MACREG 				"rtl8188E\\MAC_REG.txt"
#define RTL8188E_PHY_REG_PG				"rtl8188E\\PHY_REG_PG.txt"
#define RTL8188E_PHY_REG_MP				"rtl8188E\\PHY_REG_MP.txt"

#if 0
#define Rtl8188E_PHY_REG_Array_PG			Rtl8188EUPHY_REG_Array_PG
#define Rtl8188E_PHY_REG_Array_PGLength	Rtl8188EUPHY_REG_Array_PGLength

#endif

//---------------------------------------------------------------------
//		RTL8188E Power Configuration CMDs for USB/SDIO interfaces
//---------------------------------------------------------------------
#define Rtl8188E_NIC_PWR_ON_FLOW				rtl8188E_power_on_flow
#define Rtl8188E_NIC_RF_OFF_FLOW				rtl8188E_radio_off_flow
#define Rtl8188E_NIC_DISABLE_FLOW				rtl8188E_card_disable_flow
#define Rtl8188E_NIC_ENABLE_FLOW				rtl8188E_card_enable_flow
#define Rtl8188E_NIC_SUSPEND_FLOW				rtl8188E_suspend_flow
#define Rtl8188E_NIC_RESUME_FLOW				rtl8188E_resume_flow
#define Rtl8188E_NIC_PDN_FLOW					rtl8188E_hwpdn_flow
#define Rtl8188E_NIC_LPS_ENTER_FLOW			rtl8188E_enter_lps_flow
#define Rtl8188E_NIC_LPS_LEAVE_FLOW			rtl8188E_leave_lps_flow

#elif defined(CONFIG_PCI_HCI)
#define RTL8188E_FW_UMC_IMG				"rtl8188E\\rtl8188efw.bin"
#define RTL8188E_PHY_REG					"rtl8188E\\PHY_REG_1T.txt"
#define RTL8188E_PHY_RADIO_A				"rtl8188E\\radio_a_1T.txt"
#define RTL8188E_PHY_RADIO_B				"rtl8188E\\radio_b_1T.txt"
#define RTL8188E_AGC_TAB					"rtl8188E\\AGC_TAB_1T.txt"
#define RTL8188E_PHY_MACREG 				"rtl8188E\\MAC_REG.txt"
#define RTL8188E_PHY_REG_PG				"rtl8188E\\PHY_REG_PG.txt"
#define RTL8188E_PHY_REG_MP				"rtl8188E\\PHY_REG_MP.txt"

#define Rtl8188E_PHY_REG_Array_PG			Rtl8188EEPHY_REG_Array_PG
#define Rtl8188E_PHY_REG_Array_PGLength	Rtl8188EEPHY_REG_Array_PGLength


#ifndef CONFIG_PHY_SETTING_WITH_ODM
#if MP_DRIVER == 1
#define Rtl8188ES_PHY_REG_Array_MP 			Rtl8188ESPHY_REG_Array_MP
#endif
#endif

//---------------------------------------------------------------------
//		RTL8188E Power Configuration CMDs for USB/SDIO/PCIE interfaces
//---------------------------------------------------------------------
#define Rtl8188E_NIC_PWR_ON_FLOW				rtl8188E_power_on_flow
#define Rtl8188E_NIC_RF_OFF_FLOW				rtl8188E_radio_off_flow
#define Rtl8188E_NIC_DISABLE_FLOW				rtl8188E_card_disable_flow
#define Rtl8188E_NIC_ENABLE_FLOW				rtl8188E_card_enable_flow
#define Rtl8188E_NIC_SUSPEND_FLOW				rtl8188E_suspend_flow
#define Rtl8188E_NIC_RESUME_FLOW				rtl8188E_resume_flow
#define Rtl8188E_NIC_PDN_FLOW					rtl8188E_hwpdn_flow
#define Rtl8188E_NIC_LPS_ENTER_FLOW			rtl8188E_enter_lps_flow
#define Rtl8188E_NIC_LPS_LEAVE_FLOW			rtl8188E_leave_lps_flow
#endif //CONFIG_***_HCI

#if 1 // download firmware related data structure
#define FW_8188E_SIZE				0x4000 //16384,16k
#define FW_8188E_START_ADDRESS	0x1000
#define FW_8188E_END_ADDRESS		0x1FFF //0x5FFF




#define IS_FW_HEADER_EXIST_88E(_pFwHdr)	((le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x88E0)

typedef struct _RT_FIRMWARE_8188E {
	FIRMWARE_SOURCE	eFWSource;
#ifdef CONFIG_EMBEDDED_FWIMG
	u8			*szFwBuffer;
#else
	u8			szFwBuffer[FW_8188E_SIZE];
#endif
	u32			ulFwLength;

#ifdef CONFIG_WOWLAN
	u8			*szWoWLANFwBuffer;
	u32			ulWoWLANFwLength;
#endif //CONFIG_WOWLAN
} RT_FIRMWARE_8188E, *PRT_FIRMWARE_8188E;

//
// This structure must be cared byte-ordering
//

typedef struct _RT_8188E_FIRMWARE_HDR {
	// 8-byte alinment required

	//--- LONG WORD 0 ----
	u16		Signature;	// 92C0: test chip; 92C, 88C0: test chip; 88C1: MP A-cut; 92C1: MP A-cut
	u8		Category;	// AP/NIC and USB/PCI
	u8		Function;	// Reserved for different FW function indcation, for further use when driver needs to download different FW in different conditions
	u16		Version;		// FW Version
	u8		Subversion;	// FW Subversion, default 0x00
	u16		Rsvd1;


	//--- LONG WORD 1 ----
	u8		Month;	// Release time Month field
	u8		Date;	// Release time Date field
	u8		Hour;	// Release time Hour field
	u8		Minute;	// Release time Minute field
	u16		RamCodeSize;	// The size of RAM code
	u8		Foundry;
	u8		Rsvd2;

	//--- LONG WORD 2 ----
	u32		SvnIdx;	// The SVN entry index
	u32		Rsvd3;

	//--- LONG WORD 3 ----
	u32		Rsvd4;
	u32		Rsvd5;
} RT_8188E_FIRMWARE_HDR, *PRT_8188E_FIRMWARE_HDR;
#endif // download firmware related data structure


#define DRIVER_EARLY_INT_TIME_8188E			0x05
#define BCN_DMA_ATIME_INT_TIME_8188E		0x02


#define MAX_RX_DMA_BUFFER_SIZE_88E	      0x2400 //9k for 88E nornal chip , //MaxRxBuff=10k-max(TxReportSize(64*8), WOLPattern(16*24))

#define MAX_TX_REPORT_BUFFER_SIZE			0x0400 // 1k 


// Note: We will divide number of page equally for each queue other than public queue!
// 22k = 22528 bytes = 176 pages (@page =  128 bytes)
// must reserved about 7 pages for LPS =>  176-7 = 169 (0xA9)
// 2*BCN / 1*ps-poll / 1*null-data /1*prob_rsp /1*QOS null-data /1*BT QOS null-data

#define TX_TOTAL_PAGE_NUMBER_88E		0xA9//  169 (21632=> 21k)

#ifdef RTL8188ES_MAC_LOOPBACK
#define TX_PAGE_BOUNDARY_88E 0x48 //72
#else //TX_PAGE_BOUNDARY_LOOPBACK_MODE
#define TX_PAGE_BOUNDARY_88E (TX_TOTAL_PAGE_NUMBER_88E + 1)
#endif


//Note: For Normal Chip Setting ,modify later
#define WMM_NORMAL_TX_TOTAL_PAGE_NUMBER	TX_TOTAL_PAGE_NUMBER_88E  //0xA9 , 0xb0=>176=>22k
#define WMM_NORMAL_TX_PAGE_BOUNDARY_88E	(WMM_NORMAL_TX_TOTAL_PAGE_NUMBER + 1) //0xA9



//-------------------------------------------------------------------------
//	Chip specific
//-------------------------------------------------------------------------
#define CHIP_BONDING_IDENTIFIER(_value)	(((_value)>>22)&0x3)
#define CHIP_BONDING_92C_1T2R	0x1
#define CHIP_BONDING_88C_USB_MCARD	0x2
#define CHIP_BONDING_88C_USB_HP	0x1

//-------------------------------------------------------------------------
//	Channel Plan
//-------------------------------------------------------------------------


#define EFUSE_REAL_CONTENT_LEN		512
#define EFUSE_MAP_LEN				128
#define EFUSE_MAX_SECTION			16
#define EFUSE_IC_ID_OFFSET			506	//For some inferiority IC purpose. added by Roger, 2009.09.02.
#define AVAILABLE_EFUSE_ADDR(addr) 	(addr < EFUSE_REAL_CONTENT_LEN)
//
// <Roger_Notes>
// To prevent out of boundary programming case,
// leave 1byte and program full section
// 9bytes + 1byt + 5bytes and pre 1byte.
// For worst case:
// | 1byte|----8bytes----|1byte|--5bytes--|
// |         |            Reserved(14bytes)	      |
//
#define EFUSE_OOB_PROTECT_BYTES 		15	// PG data exclude header, dummy 6 bytes frome CP test and reserved 1byte.

#define		HWSET_MAX_SIZE_88E		512

#define		EFUSE_REAL_CONTENT_LEN_88E	256
#define		EFUSE_MAP_LEN_88E		512
#define		EFUSE_MAX_SECTION_88E		64
#define		EFUSE_MAX_WORD_UNIT_88E		4
#define		EFUSE_IC_ID_OFFSET_88E			506	//For some inferiority IC purpose. added by Roger, 2009.09.02.
#define 		AVAILABLE_EFUSE_ADDR_88E(addr) 	(addr < EFUSE_REAL_CONTENT_LEN_88E)
// <Roger_Notes> To prevent out of boundary programming case, leave 1byte and program full section
// 9bytes + 1byt + 5bytes and pre 1byte.
// For worst case:
// | 2byte|----8bytes----|1byte|--7bytes--| //92D
#define 		EFUSE_OOB_PROTECT_BYTES_88E	18 	// PG data exclude header, dummy 7 bytes frome CP test and reserved 1byte.
#define		EFUSE_PROTECT_BYTES_BANK_88E	16


//========================================================
//			EFUSE for BT definition
//========================================================
#define EFUSE_BT_REAL_CONTENT_LEN		1536	// 512*3
#define EFUSE_BT_MAP_LEN				1024	// 1k bytes
#define EFUSE_BT_MAX_SECTION			128		// 1024/8

#define EFUSE_PROTECT_BYTES_BANK		16

#define GET_RF_TYPE(priv)			(GET_HAL_DATA(priv)->rf_type)

#define INCLUDE_MULTI_FUNC_BT(_Adapter)	(GET_HAL_DATA(_Adapter)->MultiFunc & RT_MULTI_FUNC_BT)
#define INCLUDE_MULTI_FUNC_GPS(_Adapter)	(GET_HAL_DATA(_Adapter)->MultiFunc & RT_MULTI_FUNC_GPS)

//#define IS_MULTI_FUNC_CHIP(_Adapter)	(((((PHAL_DATA_TYPE)(_Adapter->HalData))->MultiFunc) & (RT_MULTI_FUNC_BT|RT_MULTI_FUNC_GPS)) ? _TRUE : _FALSE)

//#define RT_IS_FUNC_DISABLED(__pAdapter, __FuncBits) ( (__pAdapter)->DisabledFunctions & (__FuncBits) )

#ifdef CONFIG_PCI_HCI
void InterruptRecognized8188EE(PADAPTER Adapter,
			       PRT_ISR_CONTENT pIsrContent);
void UpdateInterruptMask8188EE(PADAPTER Adapter, u32 AddMSR, u32 AddMSR1,
			       u32 RemoveMSR, u32 RemoveMSR1);
#endif	//CONFIG_PCI_HCI

// rtl8188e_hal_init.c

s32 rtl8188e_FirmwareDownload(PADAPTER padapter, BOOLEAN  bUsedWoWLANFw);
void _8051Reset88E(PADAPTER padapter);
void rtl8188e_InitializeFirmwareVars(PADAPTER padapter);


s32 InitLLTTable(PADAPTER padapter, u8 txpktbuf_bndy);

// EFuse
u8 GetEEPROMSize8188E(PADAPTER padapter);
void Hal_InitPGData88E(PADAPTER padapter);
void Hal_EfuseParseIDCode88E(PADAPTER padapter, u8 *hwinfo);
void Hal_ReadTxPowerInfo88E(PADAPTER padapter, u8 *hwinfo,
			    BOOLEAN	AutoLoadFail);

void Hal_EfuseParseEEPROMVer88E(PADAPTER padapter, u8 *hwinfo,
				BOOLEAN AutoLoadFail);
void rtl8188e_EfuseParseChnlPlan(PADAPTER padapter, u8 *hwinfo,
				 BOOLEAN AutoLoadFail);
void Hal_EfuseParseCustomerID88E(PADAPTER padapter, u8 *hwinfo,
				 BOOLEAN AutoLoadFail);
void Hal_ReadAntennaDiversity88E(PADAPTER pAdapter, u8 *PROMContent,
				 BOOLEAN AutoLoadFail);
void Hal_ReadThermalMeter_88E(PADAPTER	Adapter, u8 *PROMContent,
			      BOOLEAN 	AutoloadFail);
void Hal_EfuseParseXtal_8188E(PADAPTER pAdapter, u8 *hwinfo,
			      BOOLEAN AutoLoadFail);
void Hal_EfuseParseBoardType88E(PADAPTER pAdapter, u8 *hwinfo,
				BOOLEAN AutoLoadFail);
void Hal_ReadPowerSavingMode88E(PADAPTER pAdapter, u8 *hwinfo,
				BOOLEAN AutoLoadFail);

BOOLEAN HalDetectPwrDownMode88E(PADAPTER Adapter);

#ifdef CONFIG_WOWLAN
void Hal_DetectWoWMode(PADAPTER pAdapter);
#endif //CONFIG_WOWLAN
//RT_CHANNEL_DOMAIN rtl8723a_HalMapChannelPlan(PADAPTER padapter, u8 HalChannelPlan);
//VERSION_8192C rtl8723a_ReadChipVersion(PADAPTER padapter);
//void rtl8723a_ReadBluetoothCoexistInfo(PADAPTER padapter, u8 *PROMContent, BOOLEAN AutoloadFail);
void Hal_InitChannelPlan(PADAPTER padapter);

void rtl8188e_set_hal_ops(struct hal_ops *pHalFunc);

// register
void SetBcnCtrlReg(PADAPTER padapter, u8 SetBits, u8 ClearBits);

void rtl8188e_clone_haldata(_adapter *dst_adapter, _adapter *src_adapter);
void rtl8188e_start_thread(_adapter *padapter);
void rtl8188e_stop_thread(_adapter *padapter);

void rtw_IOL_cmd_tx_pkt_buf_dump(ADAPTER *Adapter, int data_len);
#ifdef CONFIG_IOL_EFUSE_PATCH
s32 rtl8188e_iol_efuse_patch(PADAPTER padapter);
#endif//CONFIG_IOL_EFUSE_PATCH
void _InitTransferPageSize(PADAPTER padapter);
#endif //__RTL8188E_HAL_H__

