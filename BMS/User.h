/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : User.h
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2016年11月30日 星期三
  最近修改   :
  功能描述   : 用户参数定义
  函数列表   :
  修改历史   :
  1.日    期   : 2016年11月30日 星期三
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/

#ifndef __USER_H_
#define __USER_H_

#include "SysDefs.h"


#define HI_BYTE(v1_u16)              ((u8)((v1_u16)>>8))
#define LO_BYTE(v1_u16)              ((u8)((v1_u16)&0xFF))
#define BYTES_TO_INT(msb_u8,lsb_u8)	((((u16)msb_u8<<8) & 0xff00) + lsb_u8)

#define U8_MAX     (255)
#define S8_MAX     (127)
#define S8_MIN     (-128)
#define U16_MAX    (65535u)
#define S16_MAX    (32767)
#define S16_MIN    (-32768)
#define U32_MAX    (4294967295uL)
#define S32_MAX    (2147483647)
#define S32_MIN    (-2147483648uL)


#define XRAM
#ifdef __MAIN_DEF__
#define MainDef
#define MainDefIdata
#else
#define MainDef extern
#define MainDefIdata extern
#endif

#define CHG_OVER_VOL_ST                     4150
#define CHG_OVER_VOL_TOTAL_ST               (CHG_OVER_VOL_ST*CELL_NUM)
#if defined PROJ_YAO_LONG || defined PROJ_XHT
#define CHG_OVER_VOL                        4175
#define CHG_OVER_VOL_RE                     4100
#else
#define CHG_OVER_VOL                        4200
#define CHG_OVER_VOL_RE                     4050
#endif
#define CHG_OVER_VOL_TOTAL                  (CHG_OVER_VOL*CELL_NUM)
#define CHG_OVER_VOL_TOTAL_RE               (CHG_OVER_VOL_RE*CELL_NUM)
#define CHG_SMALL_CUR                       500
#define CHG_SMALL_CUR_TOTALVOL              (4150*CELL_NUM)
#define CHG_OVER_VOL2						4400
#define CHG_OVER_VOL2_RE					4075

#define CHG_OUT_CUR                         (CHG_SMALL_CUR/2) 
#define CHG_SLEEP_CUR                       (CHG_SMALL_CUR/3)  /*充电退出通讯休眠电流*/
#define DIS_SLEEP_CUR_MIN                   (DIS_UNLOAD_CUR/30*10) /*放电退出通讯休眠电流*/
#define DIS_SLEEP_CUR_MAX                   (DIS_UNLOAD_CUR/15*10)

// Ticks: 10ms
#define CHG_PLUGIN_DELAY                    35
#define CHG_PLUGOUT_DELAY                   60
#define CHG_FAIL_DELAY                  	50
#define CHG_OVER_VOL_DELAY                  100
#define CHG_SMALL_CUR_DELAY                 500
#define CHG_OVER_VOL_PRE_DELAY              20
#define CHG_OVER_VOL_RE_DELAY               200
#define CHG_OVER_VOL2_DELAY					200			// 2S
#define CHG_OVER_VOL2_RE_DELAY				200			// 2S

// Ticks: 1min
#define CHG_MAX_TIME                        720     // 12 hours
#define CHG_OVER_NTC_DISP_MAX_TIME          720     // 12 hours

//#define CHG_BALANCE_VOL                     3700    // BalanceVoltage: 3600mv
//#define CHG_BALANCE_VOL_W                   30     // BalanceVoltageWindow: 100mv
// Ticks: 10ms
//#define CHG_BALANCE_DELAY                   100

#define CHG_EXIT                            22500		// =1500*15=22500
#define CHG_VALID_HIGH                      57000UL
#define CHG_VALID_LOW                       50000UL
#define CHG_VOL_OFFSET                      2000
#define CHG_EXIT_AD							(CHG_EXIT/CHARGER_VOL_GAIN)

#define CHG_OVER_CUR                        35000	// 14A,设定值15A,isensor电流偏1A
#define CHG_OVER_CUR2                       42000	// 18A
#define CHG_OVER_CUR_AD                     480		// 16A      7mV*25/3300*4095=

#define CHG_OVER_CUR_DELAY                  400     // 2S
#define CHG_OVER_CUR2_DELAY                 200     	// 2S		--AFE COC
#define CHG_OVER_CUR_AD_DELAY               200     // 2S		--opam COC

#define DIS_UNDER_VOL                       2600
#define DIS_UNDER_VOL_RE                    2900
#define DIS_UV_UNLOAD_VOL					2700
#define DIS_UNDER_VOL_TOTAL                 (DIS_UNDER_VOL*CELL_NUM)
#define DIS_UNDER_VOL_TOTAL_RE              (DIS_UNDER_VOL_RE*CELL_NUM)
#define DIS_UV2_VOL                         2500
#define DIS_UV2_RE_VOL						2700

// Ticks: 10ms
#define DIS_UNDER_VOL_DELAY                 100      //0.5s
#define DIS_UNDER_VOL_PRE_DELAY             50      //0.5s
#define DIS_UNDER_VOL_RE_DELAY				100
#define DIS_UV_POWER_OFF_DELAY				400
#define DIS_UV2_DELAY						100
#define DIS_UV2_RE_DELAY					100

#define FACTORY_TEST_RECOVER_DELAY			50		// 500ms
#define FACTORY_TEST_UVRE					3600

#define BAT_ERROR_VALUE                     600     // 0.6v
#define BAT_FAIL_VALUE                      4500    // 4.50v
#define BAT_MAX_VALUE                       4500    // 4.50v
#define BAT_MIN_VALUE                       1800    // 1.8v
#define SDI_DIFF_VALID_VOL                  3700
#define SDI_DIFF_CELL_VOL                   300
#define CELL_VOL_CALIB_ERR_VOL				200		

// Ticks: 10ms
#define BAT_ERROR1_DELAY                    500     // 5S
#define BAT_ERROR2_DELAY                    500     // 5S   
#define BAT_ERROR3_DELAY                    1000    // 10S 
#define BAT_LOW_DELAY                       200     // 2S

#define VDD_ERR_DELAY						200		// 2S

#define DIS_1A_CUR                       	1000	// 1A
#define DIS_1A_CUR_DELAY					1000    // 10S

#if defined PROJ_YAO_LONG || defined PROJ_XHT
#define DIS_OVER_CUR1                		57000   //60A/1.5S
#define DIS_OVER_CUR2                		110000  //120A/0.5S
#else
#define DIS_OVER_CUR1                		22000
#define DIS_OVER_CUR2                		30000
#endif

#define DIS_OVER_CUR3                		155000	//			--AFE DOC1 96A/1.5s
#define DIS_OVER_CUR4                		180000	//			--AFE DOC2 180A/0.5s
#define DIS_OVER_CUR1_MOTOR1_DELAY          1300    // 13S
#define DIS_OVER_CUR2_MOTOR1_DELAY          400     // 4S
#define DIS_OVER_CUR3_MOTOR1_DELAY          15      // 0.5S		--AFE DOC1
#define DIS_OVER_CUR4_MOTOR1_DELAY          15       // 0.5S	--AFE DOC2

#define DIS_OFF_ERR_CUR						1000
#define DIS_OFF_ERR_CUR_DELAY				300

#define DIS_UNLOAD_CUR                      500
#define DIS_UNLOAD_CUR_DELAY                ((u32)30*60*1000/TIMEBASE_LOOP)		// 30min
#define DIS_WAKEUP_CUR						1000

#define NTC_DIS_HIGH                        (TMP_0C_01K+77*10)          //73℃
#define NTC_DIS_HIGH_RE                     (TMP_0C_01K+67*10)          //67℃
#define NTC_DIS_LOW_RE                      (TMP_0C_01K-12*10)          //-12℃
#define NTC_DIS_LOW                         (TMP_0C_01K-17*10)          //-17℃


#define NTC_CHG_HIGH                        (TMP_0C_01K+57*10)      //57℃
#define NTC_CHG_HIGH_RE                     (TMP_0C_01K+47*10)      //47℃

#define NTC_CHG_LOW_RE                      (TMP_0C_01K+4*10)       //4℃
#define NTC_CHG_LOW                         (TMP_0C_01K+3*10)       //3℃

#define NTC_CMOS_HIGH						(TMP_0C_01K+95*10)
#define NTC_CMOS_HIGH_RE					(TMP_0C_01K+75*10)

#define NTC_DMOS_HIGH						(TMP_0C_01K+95*10)
#define NTC_DMOS_HIGH_RE					(TMP_0C_01K+75*10)

#define NTC_MOS_SHORT						(TMP_0C_01K+120*10)
#define NTC_MOS_OPEN						(TMP_0C_01K-30*10)
#define NTC_CELL_SHORT						(TMP_0C_01K+110*10)
#define NTC_CELL_OPEN						(TMP_0C_01K-30*10)

#define NTC_BAT_P30							(TMP_0C_01K+30*10)	
#define NTC_BAT_P10							(TMP_0C_01K+10*10)

// Ticks: 10ms
#define NTC_ERROR_DELAY                     200         // 2S     
#define NTC_FAIL_TIME                       500         // 5S
#define DIS_NTC_OT_DELAY                    200
#define CHG_NTC_OT_DELAY                    200
#define CHG_NTC_OT_PRE_DELAY                80
#define DIS_NTC_OT_RE_DELAY                 200
#define CHG_NTC_OT_RE_DELAY                 200
#define MOS_NTC_OT_DELAY					200

// LED Timer
// Ticks: 10ms
#define LED_10S_TIME                        1000
#define LED_1S_TIME                         100
#define LED_6S_TIME                         600
#define LED_5S_TIME                         500     // 1sec*5
#define LED_5MIN_TIME                       30000   // 30000*10ms = 300S = 5Min
#define LED_500MS_TIME                      50
#define LED_1000MS_TIME                     100
#define LED_1500MS_TIME                     150
#define TEST_5MIN_TIME                      30000   //30000*10ms = 300s =5min
#define LED_KEY_TIME						500		// 5S
// Ticks: 1s
#define LED_5MIN_CNT                        300     // 300*1s = 5min
#define LED_10MIN_CNT                       600     // 600*1s = 10min
#define LED_10SEC_CNT                       10      // 10*1s = 10sec

// Ticks: 10ms
#define POWER_OFF_REQ_CHI_DELAY             6000     // 60sec //预关机延时
#define POWER_OFF_REQ_DELAY                 5900     // 60sec     //预关机延时
#define POWER_OFF_DELAY                     150      // 1500ms    //关机延时
#define SLEEP_TO_WAKEUP_TIME				(3000U/TIMEBASE_LOOP) //(120/TIMEBASE_LOOP)			// 0.12S
#define	ENTER_SLEEP_NORMAL_DELAY			(30UL*60*1000/TIMEBASE_LOOP)	// 30min
#define ENTER_SLEEP_DSGOFF_DELAY			(30UL*1000/TIMEBASE_LOOP)		// 30s
#define COMOVER_DELAY                       (3000/TIMEBASE_LOOP)        /*无任意通讯无电流3s进入休眠*/
#define NOMAINCOM_DELAY                     (24000/TIMEBASE_LOOP)       /*无主板通讯无电流24s停止并包通讯*/
#define NOMAINCOM_LOADDELAY                 (120000/TIMEBASE_LOOP)      /*无主板通讯无电流曾经大电流过则120s停止并包通讯*/

/*----------------Dis-Charge Current----------------*/
#define M1_CUR_ZERO_DEFAULT     1356           							// M1零飘范围默认值 1.6556/5000*4095
#define M1_CUR_ZERO_MAX         (M1_CUR_ZERO_DEFAULT+200)               // M1零飘范围最大值 
#define M1_CUR_ZERO_MIN         (M1_CUR_ZERO_DEFAULT-200)               // M1零飘范围最小值  

#define M2_CUR_ZERO_DEFAULT     (u16)(4095UL*21/1001)           		// M2零飘范围默认值 4095/521*50=
#define M2_CUR_ZERO_MAX         (M2_CUR_ZERO_DEFAULT+200)               // M2零飘范围最大值 
#define M2_CUR_ZERO_MIN         (M2_CUR_ZERO_DEFAULT-50)               	// M2零飘范围最小值 

/*----------------Charge Current----------------*/
#define CHG_CUR_ZERO_DEFAULT    1969               						// 充电零飘范围默认值 3300/52 *25 / 3300*4095
#define CHG_CUR_ZERO_MAX        (CHG_CUR_ZERO_DEFAULT+620)             // 充电零飘范围最大值 20mV*25 / 3300*4095 = 620
#define CHG_CUR_ZERO_MIN        (CHG_CUR_ZERO_DEFAULT-620)             // 充电零飘范围最小值  

#pragma pack(1)						// 按1字节对齐
typedef struct
{
    u8 DisProt               : 8;    /* Byte1 */
    u8 ChgProt               : 8;    /* Byte2 */
    u8 HardFault             : 8;    /* Byte3 */    
    u16 BmsState             : 16;	 /* Byte4-5 */
    u16 InputState              : 16;    /* Byte6 */
} StatusByte;

typedef struct
{
    /* Byte1 */    
    u8 DisUV            : 1;    /* 放电过放状态         1: 过放         0: 未过放 */
    u8 DisDUV           : 1;    /* 放电深度过放状态     1: 深度过放     0: 未深度过放 */
    u8 DisOC          	: 1;    /* 放电过流状态    		1: 过流     	0: 未过流 */
    u8 DisOCL          	: 1;    /* 放电过流锁    		1: 过流     	0: 未过流 */
    //-------------------------------------------------------------------------------
    u8 DisSC            : 1;    /* 放电短路状态         1: 短路         0: 未短路 */    
    u8 DisOT        	: 1;    /* 放电过温状态     	1: 过温         0: 未过温 */   
    u8 DisUL            : 1;	/* 放电空载状态     	1: 空载         0: 正常   */
    u8 DisMOT			: 1;	/* MOS过温状态     		1: 过温         0: 未过温 */
    
    //==============================================================================
    
    /* Byte2 */
    u8 ChgOV            : 1;    /* 充电过压状态         1: 充电过压     0: 未过压 */    
    u8 ChgOC            : 1;    /* 充电过流状态         1: 充电过流     0: 未过流 */
    u8 ChgTO       		: 1;    /* 充电超时状态         1: 超时         0: 未超时 */
    u8 ChgER            : 1;    /* 充电器故障           1: 故障         0: 正常 */
    //-------------------------------------------------------------------------------
    u8 ChgOT            : 1;    /* 充电过温状态         1: 充电过温     0: 未过温 */
    u8 ChgMOT           : 1;    /* 充电MOS过温状态     	1: 过温         0: 未过温 */    
    u8 ChgStop          : 1;    /* 充电停止状态   		1: 停止         0: 正常 */
    u8 ChgFC	        : 1;    /* 充电满充状态         1: 满充         0: 未满充 */  
    
	//===============================================================================
	
    /* Byte3 */
    u8 CellNtcErr       : 1;    /* 电芯NTC状态          1: NTC故障      0: 正常 */
    u8 DNtcErr          : 1;    /* 放电MOS NTC状态      1: NTC故障      0: 正常 */    
    u8 AfeErr          	: 1;    /* AFE故障状态          1: 故障         0: 正常 */
	u8 CMosErr          : 1;    /* 充电MOS短路状态      1: 故障         0: 正常 */
    //-------------------------------------------------------------------------------
    u8 VddErr           : 1;    /* Vdd电压状态          1: 故障         0: 正常   */    
    u8 BatErr           : 1;    /* 电芯故障状态         1: 电芯故障     0: 电芯正常 */
    u8 DMosErr          : 1;    /* 放电MOS短路状态      1: 故障         0: 正常 */
    u8 CNtcErr          : 1;    /* 充电MOS NTC状态      1: NTC故障      0: 正常 */
	//===============================================================================

    /* Byte4-5 */
    u8 Dis              : 1;    /* 放电状态             1: 放电         0: 未放电 */
    u8 Chg              : 1;    /* 充电状态             1: 充电         0: 未充电 */
    u8 Dmos				: 1;
    u8 Cmos				: 1;
    //-------------------------------------------------------------------------------
    u8 AfeDsg			: 1;
    u8 AfeChg      		: 1;
    u8 PDmos			: 1;
    u8 PCmos			: 1;
    
    //-------------------------------------------------------------------------------
    u8 PowerOff         : 1;    /* 关机请求             1: 请求         0: 未请求 */
    u8 ShutDown         : 1;    /* 强制关机状态         1: 请求         0: 未请求 */
    u8 Sleep            : 1;	/* 状态         		1:              0: 正常 */
    u8 CanErr					  : 1;
    //-------------------------------------------------------------------------------
    u8 CanTO			: 1;    //CAN通讯超时
    u8 CanAlone			: 1;	/* can状态       		1: 孤立         0: 运行 */
		u8 DsgAble    : 1;
    u8	ChgAble				: 1;
 
    //===============================================================================    
    /* Byte6 */
    u8 KeyOn            : 1;    /* Key ON状态           1: Key ON       0: Key OFF */ 
    u8 Active			: 1;    /* Key led状态          1: 电量显示     0: 不显示 */
    u8 DetIn            : 1;    /* 电池包在位状态       1: 在位         0: 不在位 */ 
    u8 ChgPlugin        : 1;    /* 充电器插入状态       1: 插入         0: 移除 */
    //-------------------------------------------------------------------------------
    u8 LoadOn			: 1;	/* Afe检测负载接入    	1: 已接入了   	0: x-未知 */	
		u8 LoadOff			: 1;	/* Afe检测负载移除     	1: 已移除了    	0: x-未知 */
    u8 chgerAction      : 1;    //充电器激活，但又达不到充电状态
    u8 OnStatus         : 1;    //用来表示开机
		
		u8             : 1;    /* 电池包在位状态       1: 在位         0: 不在位 */ 
		u8 ChgOV2 				:1;
		u8 DetIn_Test     :1;
    u8               :5;
}StatusBit;


typedef union
{
    StatusByte Bytes;
    StatusBit Bits;
} BmsStatus_T;
MainDefIdata BmsStatus_T BatStatus;
#define CHG_COT_MASK			0x10
#define CHG_MOT_MASK			0x20
#define CHG_DOT_MASK			0x40
#define CHG_ONLY_OT_MASK		0x70
#define CHG_ONLY_OT_IVMASK		(~CHG_ONLY_OT_MASK)
#define CHG_ONLY_WAIT_STATE()	(((BatStatus.Bytes.ChgProt!=0) && ((BatStatus.Bytes.ChgProt&CHG_ONLY_OT_IVMASK)==0x00)) && (BatStatus.Bytes.HardFault==0x00))
#define CHG_ONLY_OT_STATE()		(((BatStatus.Bytes.ChgProt!=0) && ((BatStatus.Bytes.ChgProt&CHG_ONLY_OT_IVMASK)==0x00)) && (BatStatus.Bytes.HardFault==0x00))
#define IS_CHG_STATE()         ((BatStatus.Bits.ChgPlugin==1)&&(ChgCurAvg>CHG_SMALL_CUR)&&(BatStatus.Bits.Chg))  
#define DevRegsMax      100     //最大寄存器数量
typedef union DevRegBuf DevRegBuf;
union DevRegBuf
{
    uint16_t buf[DevRegsMax];
    struct 
    {
        char        hwVer[16];      //硬件版本 寄存器地址0 ,16字节, 长度 8个寄存器
        char        swVer[16];      //软件版本 寄存器地址8 ,16字节, 长度 8个寄存器
        uint16_t    subVer;         //子版本 寄存器地址16 ,2字节, 长度 1个寄存器
        BmsStatus_T bmsStatus;      //电池状态 寄存器地址17,6字节,长度3个寄存器
        uint32_t    packVolt;       //pack电压 寄存器地址20 ,4字节, 长度 2个寄存器
        uint32_t    chgerVolt;      //充电器电压 寄存器地址22 ,4字节, 长度 2个寄存器
        uint16_t    cell[CELL_NUM]; //电芯电压 寄存器地址24 ,26字节, 长度 CELL_NUM 15个寄存器
        uint32_t    disCur;         //放电电流 寄存器地址39,4字节,长度 2个寄存器
        uint32_t    chgCur;         //充电电流 寄存器地址41,4字节,长度 2个寄存器
        uint16_t    ntcTemp[4];     //ntc温度 寄存器地址43 ,8字节,长度 4个寄存器
        uint16_t    soc;           //soc值 寄存器地址47 ,2字节,长度 1个寄存器
        uint32_t    disCheckCur;         //放电电流 寄存器地址49,4字节,长度 2个寄存器
        uint32_t    chgCheckCur;         //充电电流 寄存器地址51,4字节,长度 2个寄存器
        uint16_t    cellNtc[2];         //电芯温度 寄存器地址53,4字节,长度 2个寄存器
    }reg;
};

#pragma pack()						//  取消按1字节对齐

typedef struct DevAutoReportReg DevAutoReportReg;
struct DevAutoReportReg
{
    uint8_t     autoReportFlag;     //0,没有上报,1串口上报,2 Can上报
    uint8_t     regNum;     //寄存器数量
    uint16_t    regAddr;    //寄存器地址
};
typedef struct
{
    u16 Vol;
    u16 Vol_AD;
    u16 KiData;
} Cell_Data_T;
typedef struct
{
	u8  Valid;
	u16 Vdd;
    u16 Vcc;
    u16 VolMin;
    u16 VolMax;
    u16 VolAvg;
    u8 VolMinIndex;
    u8 VolMaxIndex;
    u32 VolTotal;
    u32 VolPack;
    u32 VolStack;
    Cell_Data_T Bat[CELL_NUM];
    s32  cur;
    s32  cadc;
} BatData_T;

MainDef BatData_T BatData;

typedef enum
{
    EVENT_NONE = 0,
    EVENT_KEYPRE,
    EVENT_KEYDOWN,
    EVENT_CMDKEY,
    EVENT_CMDSCL,
    EVENT_CMDCHG,
    EVENT_CHGIN,
    EVENT_CHGOUT
} EventEnum;
MainDef EventEnum UIEvent;

typedef enum
{
	STA_SHUTDOWN = 0,
	STA_WAIT,
	STA_WORK,
	STA_CHG,
	STA_SLEEP	
}BMSState_E;
MainDef BMSState_E	BmsState;

//MainDefIdata u16 ChgCurrent_K;                  // 充电电流校准系数
//MainDefIdata u16 DisCurrent_K;                  // 放电电流校准系数
//MainDef u8 BatErrorCnt;                     // 记录电芯损坏次数
MainDef u8 TestLedFlag;                     // 测试模式中,控制LED ALL ON/OFF
MainDef u32 ChgAvgVol;                         // 充电器电压AD值
//MainDef u16 ChgCurAD;                       // 充电电流AD值
//MainDef u16 Motor1CurAD;                    // 放电电流AD值

//MainDef u8 BatIndex;                        // 电芯采样切换通道值
MainDef u16 BatError1Timer;                 // 电芯故障1检测计时器
MainDef u16 BatError2Timer;                 // 电芯故障2检测计时器
MainDef u16 BatError3Timer;                 // 电芯故障3检测计时器
MainDef u16 DisUnderVolTimer[CELL_NUM];      // 过放检测计时器

MainDefIdata u32 DisCurAvg;           // 马达1电流值
MainDefIdata u32 DisCurAvg;      // 马达1电流检查值
MainDef u16 DisOverCurTimer[4];          // 马达1过流计时器

MainDef u16 ChgMinTimer;					// 充电总时长分针计时器
MainDef u16 ChgSecTimer;                    // 充电总时长秒针计时器
MainDef u16 ChgOverNtcDispSecTimer;         // 充电假充时长秒针计时器
MainDef u16 ChgOverVolTimer[2];              // 充电单节过充计时器
MainDef u16 ChgOverVolTotalTimer;           // 充电总电压过充计时器
MainDef u16 ChgOverVolReTimer[2];            // 过充恢复计时器
MainDefIdata u32 ChgCurAvg;                 // 充电电流值
MainDefIdata u32 ChgCurAvg;                 // 充电电流检查值
MainDef u16 ChgOverCurTimer[3];             // 充电过流计时器
MainDef u16 ChgSmallCurTimer;               // 充电小电流计时器
MainDef u16 ChgCurZero;                     // 充电电流零漂值
MainDef u16 ChgCurAD;						// 充电电流AD值
MainDef u16 ChgCurAvgAD;					// 充电电流AD滤波值
MainDef u16 NtcAvg;                         // NTC检测值
MainDef u8 ChgOV2;							//充电过压2

MainDef u16 CbState;
MainDef u32 PackAvgVol;                  	// Pack端电压(单位:mV)

MainDef u16 DmosNtcTemp;
MainDef u16 CmosNtcTemp;
MainDef u16 DiodeNtcTemp;
MainDef u16 MosNtcTemp[MOS_NTC_NUM];
MainDef u16 MosNtcTempMax;

MainDef u16 BatNtcTemp[BAT_NTC_NUM];
MainDef u16 BatNtcTempMax;
MainDef u16 BatNtcTempMin;

MainDef u16 NtcDisTimer;                     // 放电NTC过温计时器
MainDef u16 NtcDisReTimer;                   // 放电NTC过温恢复计时器
MainDef u16 NtcChgTimer;                     // 充电NTC过温计时器
MainDef u16 NtcChgReTimer;                   // 充电NTC过温恢复计时器
//MainDef u16 NtcFailTimer;                   // NTC故障计时器
MainDef volatile u8 SysTickTimer;           // 1MS基准定时器

MainDefIdata bit TimerBatFlag;                   // 电芯电压检测基准定时器标识
MainDefIdata bit TimerNtcFlag;                   // NTC检测基准定时器标识
MainDefIdata bit TimerDisFlag;                   // 放电检测基准定时器标识
MainDefIdata bit TimerChgFlag;                   // 充电检测基准定时器标识
MainDefIdata bit TimerKeyFlag;                   // 按键检测基准定时器标识
MainDefIdata bit TimerLedFlag;                   // LED控制基准定时器标识
MainDefIdata bit TimerMiscFlag;                  // 测试模式定时器标识
MainDefIdata bit TimerUartBWFlag;                // UART基准计时器
MainDefIdata bit TimerUartCTFlag;                // UART基准计时器
MainDefIdata bit CalibNgFlag;
//MainDefIdata bit BatValidFlag;
MainDefIdata bit TimerDataLogFlag;

MainDef u8 FactoryTestMode;                 // 生产测试模式
MainDef u16 AteForceCtrlTimer;				// 产测强制控制计时器
MainDef u16 LedTestSet;
MainDef u32	FactoryBalanceSelect;
MainDef u8	FactoryBalanceEnable;


MainDef u16 ZnyTest1;
MainDef u16 ZnyTest2;
MainDef u16 ZnyTest3;
MainDef u16 ZnyTest4;
MainDef u16 ZnyTest5;

MainDef u16 PowerOffTimer;
MainDef u16 ShutDownTimer;
MainDef u32 EnterSleepTimer;
MainDef u16 ComOverSleepTimer; 
MainDef u16 Dis_Sleep_Cur_V;
MainDef u8 SleepOnMark;
MainDef u8 SingleMosOnMark;/*单包开mos的标志，用于唤醒后不识别canerr直到id移除*/
MainDef u8 chgcurlock; /*不能充电出现充电电流锁标记*/

MainDef volatile u8 Rtc1STick;

MainDef volatile u16 SlUpgradeTimer;
#define BMS_IS_UPGRADING()			( SlUpgradeTimer > 0 ) 


#ifdef UART_DEBUG
MainDef u16 UartDebugCnt;
#endif

#define IS_FACTORY_TEST_MODE()		(FactoryTestMode==0x53)
#define ENTER_FACTORY_TEST_MODE()	{FactoryTestMode=0x53;}
#define EXIT_FACTORY_TEST_MODE()	{FactoryTestMode=0x00;}

#define IS_FCT_FORCE_CTRL()			((AteForceCtrlTimer>0)&&(FactoryTestMode==0x53))


//extern void DisSCCheck(void);
//extern void TimeBaseManager(void);
//extern void BootReceive(u8 rx_data);

//extern void BatCellSel(u8 nCell);
extern void VarTimerReset(void);


#endif
