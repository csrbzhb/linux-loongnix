//===========================================
// The following is for 8723B 1ANT BT Co-exist definition
//===========================================
#define	BT_AUTO_REPORT_ONLY_8723B_1ANT				1

#define	BT_INFO_8723B_1ANT_B_FTP						BIT7
#define	BT_INFO_8723B_1ANT_B_A2DP					BIT6
#define	BT_INFO_8723B_1ANT_B_HID						BIT5
#define	BT_INFO_8723B_1ANT_B_SCO_BUSY				BIT4
#define	BT_INFO_8723B_1ANT_B_ACL_BUSY				BIT3
#define	BT_INFO_8723B_1ANT_B_INQ_PAGE				BIT2
#define	BT_INFO_8723B_1ANT_B_SCO_ESCO				BIT1
#define	BT_INFO_8723B_1ANT_B_CONNECTION				BIT0

#define	BT_INFO_8723B_1ANT_A2DP_BASIC_RATE(_BT_INFO_EXT_)	\
		(((_BT_INFO_EXT_&BIT0))? TRUE:FALSE)

#define	BTC_RSSI_COEX_THRESH_TOL_8723B_1ANT		2

#define  BT_8723B_1ANT_WIFI_NOISY_THRESH								30   //max: 255

typedef enum _BT_INFO_SRC_8723B_1ANT {
	BT_INFO_SRC_8723B_1ANT_WIFI_FW			= 0x0,
	BT_INFO_SRC_8723B_1ANT_BT_RSP				= 0x1,
	BT_INFO_SRC_8723B_1ANT_BT_ACTIVE_SEND		= 0x2,
	BT_INFO_SRC_8723B_1ANT_MAX
} BT_INFO_SRC_8723B_1ANT, *PBT_INFO_SRC_8723B_1ANT;

typedef enum _BT_8723B_1ANT_BT_STATUS {
	BT_8723B_1ANT_BT_STATUS_NON_CONNECTED_IDLE	= 0x0,
	BT_8723B_1ANT_BT_STATUS_CONNECTED_IDLE		= 0x1,
	BT_8723B_1ANT_BT_STATUS_INQ_PAGE				= 0x2,
	BT_8723B_1ANT_BT_STATUS_ACL_BUSY				= 0x3,
	BT_8723B_1ANT_BT_STATUS_SCO_BUSY				= 0x4,
	BT_8723B_1ANT_BT_STATUS_ACL_SCO_BUSY			= 0x5,
	BT_8723B_1ANT_BT_STATUS_MAX
} BT_8723B_1ANT_BT_STATUS, *PBT_8723B_1ANT_BT_STATUS;

typedef enum _BT_8723B_1ANT_WIFI_STATUS {
	BT_8723B_1ANT_WIFI_STATUS_NON_CONNECTED_IDLE				= 0x0,
	BT_8723B_1ANT_WIFI_STATUS_NON_CONNECTED_ASSO_AUTH_SCAN		= 0x1,
	BT_8723B_1ANT_WIFI_STATUS_CONNECTED_SCAN					= 0x2,
	BT_8723B_1ANT_WIFI_STATUS_CONNECTED_SPECIAL_PKT				= 0x3,
	BT_8723B_1ANT_WIFI_STATUS_CONNECTED_IDLE					= 0x4,
	BT_8723B_1ANT_WIFI_STATUS_CONNECTED_BUSY					= 0x5,
	BT_8723B_1ANT_WIFI_STATUS_MAX
} BT_8723B_1ANT_WIFI_STATUS, *PBT_8723B_1ANT_WIFI_STATUS;

typedef enum _BT_8723B_1ANT_COEX_ALGO {
	BT_8723B_1ANT_COEX_ALGO_UNDEFINED			= 0x0,
	BT_8723B_1ANT_COEX_ALGO_SCO				= 0x1,
	BT_8723B_1ANT_COEX_ALGO_HID				= 0x2,
	BT_8723B_1ANT_COEX_ALGO_A2DP				= 0x3,
	BT_8723B_1ANT_COEX_ALGO_A2DP_PANHS		= 0x4,
	BT_8723B_1ANT_COEX_ALGO_PANEDR			= 0x5,
	BT_8723B_1ANT_COEX_ALGO_PANHS			= 0x6,
	BT_8723B_1ANT_COEX_ALGO_PANEDR_A2DP		= 0x7,
	BT_8723B_1ANT_COEX_ALGO_PANEDR_HID		= 0x8,
	BT_8723B_1ANT_COEX_ALGO_HID_A2DP_PANEDR	= 0x9,
	BT_8723B_1ANT_COEX_ALGO_HID_A2DP			= 0xa,
	BT_8723B_1ANT_COEX_ALGO_MAX				= 0xb,
} BT_8723B_1ANT_COEX_ALGO, *PBT_8723B_1ANT_COEX_ALGO;

typedef struct _COEX_DM_8723B_1ANT {
	// hw setting
	u1Byte		preAntPosType;
	u1Byte		curAntPosType;
	// fw mechanism
	BOOLEAN		bCurIgnoreWlanAct;
	BOOLEAN		bPreIgnoreWlanAct;
	u1Byte		prePsTdma;
	u1Byte		curPsTdma;
	u1Byte		psTdmaPara[5];
	u1Byte		psTdmaDuAdjType;
	BOOLEAN		bAutoTdmaAdjust;
	BOOLEAN		bPrePsTdmaOn;
	BOOLEAN		bCurPsTdmaOn;
	BOOLEAN		bPreBtAutoReport;
	BOOLEAN		bCurBtAutoReport;
	u1Byte		preLps;
	u1Byte		curLps;
	u1Byte		preRpwm;
	u1Byte		curRpwm;

	// sw mechanism
	BOOLEAN 	bPreLowPenaltyRa;
	BOOLEAN		bCurLowPenaltyRa;
	u4Byte		preVal0x6c0;
	u4Byte		curVal0x6c0;
	u4Byte		preVal0x6c4;
	u4Byte		curVal0x6c4;
	u4Byte		preVal0x6c8;
	u4Byte		curVal0x6c8;
	u1Byte		preVal0x6cc;
	u1Byte		curVal0x6cc;
	BOOLEAN		bLimitedDig;

	u4Byte		backupArfrCnt1;	// Auto Rate Fallback Retry cnt
	u4Byte		backupArfrCnt2;	// Auto Rate Fallback Retry cnt
	u2Byte		backupRetryLimit;
	u1Byte		backupAmpduMaxTime;

	// algorithm related
	u1Byte		preAlgorithm;
	u1Byte		curAlgorithm;
	u1Byte		btStatus;
	u1Byte		wifiChnlInfo[3];

	u4Byte		preRaMask;
	u4Byte		curRaMask;
	u1Byte		preArfrType;
	u1Byte		curArfrType;
	u1Byte		preRetryLimitType;
	u1Byte		curRetryLimitType;
	u1Byte		preAmpduTimeType;
	u1Byte		curAmpduTimeType;
	u4Byte		nArpCnt;

	u1Byte		errorCondition;
} COEX_DM_8723B_1ANT, *PCOEX_DM_8723B_1ANT;

typedef struct _COEX_STA_8723B_1ANT {
	BOOLEAN					bBtLinkExist;
	BOOLEAN					bScoExist;
	BOOLEAN					bA2dpExist;
	BOOLEAN					bHidExist;
	BOOLEAN					bPanExist;
	BOOLEAN					bBtHiPriLinkExist;

	BOOLEAN					bUnderLps;
	BOOLEAN					bUnderIps;
	u4Byte					specialPktPeriodCnt;
	u4Byte					highPriorityTx;
	u4Byte					highPriorityRx;
	u4Byte					lowPriorityTx;
	u4Byte					lowPriorityRx;
	s1Byte					btRssi;
	BOOLEAN					bBtTxRxMask;
	u1Byte					preBtRssiState;
	u1Byte					preWifiRssiState[4];
	BOOLEAN					bC2hBtInfoReqSent;
	u1Byte					btInfoC2h[BT_INFO_SRC_8723B_1ANT_MAX][10];
	u4Byte					btInfoC2hCnt[BT_INFO_SRC_8723B_1ANT_MAX];
	BOOLEAN					bBtWhckTest;
	BOOLEAN					bC2hBtInquiryPage;
	BOOLEAN					bC2hBtPage;				//Add for win8.1 page out issue
	BOOLEAN					bWiFiIsHighPriTask;		//Add for win8.1 page out issue
	u1Byte					btRetryCnt;
	u1Byte					btInfoExt;
	u4Byte					popEventCnt;
	u1Byte					nScanAPNum;

	u4Byte					nCRCOK_CCK;
	u4Byte					nCRCOK_11g;
	u4Byte					nCRCOK_11n;
	u4Byte					nCRCOK_11nAgg;

	u4Byte					nCRCErr_CCK;
	u4Byte					nCRCErr_11g;
	u4Byte					nCRCErr_11n;
	u4Byte					nCRCErr_11nAgg;

	BOOLEAN					bCCKLock;
	BOOLEAN					bPreCCKLock;
	BOOLEAN					bCCKEverLock;
	u1Byte					nCoexTableType;

	BOOLEAN					bForceLpsOn;
} COEX_STA_8723B_1ANT, *PCOEX_STA_8723B_1ANT;

#define  BT_8723B_1ANT_ANTDET_PSD_POINTS			256	//MAX:1024
#define  BT_8723B_1ANT_ANTDET_PSD_AVGNUM		1	//MAX:3

typedef struct _PSDSCAN_STA_8723B_1ANT {

	BOOLEAN			bIsAntDetEnable;
	BOOLEAN			bIsAntIsoEnable;
	BOOLEAN			bIsPSDScanEnable;

	u4Byte		 	realcentFreq;  //ex:2412
	s4Byte			realoffset;
	u4Byte			realspan;
	u4Byte			realseconds;

	BOOLEAN			bAntDetFinish;
	u1Byte			nAntIsolation;
	u4Byte			nPSDBandWidth;  //unit: Hz
	u4Byte			nPSDPoint;		//128/256/512/1024
	u4Byte			nPSDReport[1024];  //unit:dB (20logx), 0~255
	u4Byte			nPSDReport_MaxHold[1024];  //unit:dB (20logx), 0~255
	u4Byte			nPSDStartPoint;
	u4Byte			nPSDStopPoint;
	u4Byte			nPSDMaxValuePoint;
	u4Byte			nPSDMaxValue;
	u4Byte			nPSDStartBase;
	u4Byte			nPSDAvgNum;	// 1/8/16/32
	u4Byte			nPSDGenCount;
	u4Byte			nPSDGenTotalCount;
	BOOLEAN			bIsSetupFinish;
	BOOLEAN			bIsPSDRunning;
	BOOLEAN			bIsPSDShowMaxOnly;
} PSDSCAN_STA_8723B_1ANT, *PPSDSCAN_STA_8723B_1ANT;

//===========================================
// The following is interface which will notify coex module.
//===========================================
VOID EXhalbtc8723b1ant_PowerOnSetting(
	IN	PBTC_COEXIST		pBtCoexist
);
VOID EXhalbtc8723b1ant_PreLoadFirmware(
	IN	PBTC_COEXIST		pBtCoexist
);
VOID EXhalbtc8723b1ant_InitHwConfig(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	BOOLEAN				bWifiOnly
);
VOID EXhalbtc8723b1ant_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
);
VOID EXhalbtc8723b1ant_IpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
);
VOID EXhalbtc8723b1ant_LpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
);
VOID EXhalbtc8723b1ant_ScanNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
);
VOID EXhalbtc8723b1ant_ConnectNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
);
VOID EXhalbtc8723b1ant_MediaStatusNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
);
VOID EXhalbtc8723b1ant_SpecialPacketNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				type
);
VOID EXhalbtc8723b1ant_BtInfoNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	pu1Byte			tmpBuf,
	IN	u1Byte			length
);
VOID EXhalbtc8723b1ant_RfStatusNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte					type
);
VOID EXhalbtc8723b1ant_HaltNotify(
	IN	PBTC_COEXIST			pBtCoexist
);
VOID EXhalbtc8723b1ant_PnpNotify(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				pnpState
);
VOID EXhalbtc8723b1ant_CoexDmReset(
	IN	PBTC_COEXIST			pBtCoexist
);
VOID EXhalbtc8723b1ant_Periodical(
	IN	PBTC_COEXIST			pBtCoexist
);
VOID EXhalbtc8723b1ant_DisplayCoexInfo(
	IN	PBTC_COEXIST		pBtCoexist
);
VOID EXhalbtc8723b1ant_AntennaDetection(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u4Byte					centFreq,
	IN	u4Byte					offset,
	IN	u4Byte					span,
	IN	u4Byte					seconds
);
VOID EXhalbtc8723b1ant_AntennaIsolation(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u4Byte					centFreq,
	IN	u4Byte					offset,
	IN	u4Byte					span,
	IN	u4Byte					seconds
);

VOID EXhalbtc8723b1ant_PSDScan(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u4Byte					centFreq,
	IN	u4Byte					offset,
	IN	u4Byte					span,
	IN	u4Byte					seconds
);
VOID EXhalbtc8723b1ant_DisplayAntIsolation(
	IN	PBTC_COEXIST			pBtCoexist
);
