#ifndef __DATAFLASH__H__
#define __DATAFLASH__H__

#include "McuHal.h"
#include "user.h"


#define UV_LOCKED_MARK			0xA0
#define UV_UNLOCK_MARK			0x50
#define OV_LOCKED_MARK			0x53
#define FL_LOCKED_MARK			0x51
#define OT_LOCKED_MARK			0xA1

#define POWEROFF_MARK           0xAA
#define POWERON_MARK            0x55

#define MAX_CUR_K				12000
#define MIN_CUR_K				8000
#define DEFAULT_CUR_K			10000

#define MAX_CELLVOL_K			11000
#define MIN_CELLVOL_K			9000
#define DEFAULT_CELLVOL_K		10000

#define PRODUCT_ACTIVED			0x35

#define DATAFLASH_BASE_ADDR		(0)							// 预留程序空间最后2K
#define AT_ONCE_SAVE_ADDR		(DATAFLASH_BASE_ADDR+0)	// 立即保存的数据首址
#define POWER_DOWN_SAVE_ADDR	(DATAFLASH_BASE_ADDR+FLASH_PAGE_SIZE)	// 掉电时保存的数据首址，两部分数据不放在一个扇区比较安全


#if defined ( __CC_ARM   )
#pragma pack(4)						// 按4字节对齐，因为Flash擦写均以4字节对齐
#endif

typedef struct
{
    u8 Ocv;         /* OpenCircuitVoltage           基于开路电压进行SOC检测标识 */
    u8 Soc;         /* StateOfCharge */
    u8 SocS;        /* State of charge use for display, after smooth regulatar */
    u8 Soh;         /* StateOfHealth */
    u16 DisCap;     /* mA*H, DischargeCapacity:     电芯放出容量统计，用于SOH计算 */
    u16 Fcc;        /* mA*H, FullChargeCapacity:    电芯满充容量 */
    u16 Rmc;        /* mA*H，RemainingCapacity:     电芯剩余容量 */
    u16 Froc;       /* mA*H, FrozenCapacity:        电芯冻结容量 */
    u16 Cycle;      /* 电芯循环次数                              */
    u32 SocImAms;               /* mA*ms, mAH积分余数，换算参见MAMS_TO_MAH定义 */
    u32 SocImAmsDisp;
    u16 RmcDisp;
} LogSocStrc;       /* total bytes = 14 */

typedef enum
{
	K_CHG_CUR = 0,
	K_DSGM_CUR,
	K_CUR_NUM
}Cur_Calib_K;

//立即保存的数据结构，全局变量，外部引用修改
typedef struct
{
		u16 VPackKi;					/* Pout+电压校准K								*/
#ifndef AFE_MODEULE
    u16 BAT_K[CELL_NUM];	// 电压校准系数
#endif
	u16 Current_K[K_CUR_NUM];		// 充电电流校准系数
#ifdef AFE_MODEULE
    s32 AfeMCBias;
    s32 AfeSCBias;
#endif
    u8  QRCode[32];			// PCM二维码
    u8  SNCode[64];			// PACK二维码
    u16 VddData;
    u16 VddVerify;
	  u16 OpaBias;   /*运放偏置*/
    u32 Crc;
} StrDataFlashAtOnceSave;

//掉电保存的数据结构，全局变量，外部引用修改
typedef struct
{
	u32 BootMark;				// bootloader标识，需要放置在首位
	u32 UpgradeLen;
	u32 UpgradeCrc;
	
    u8 PowerMark;               // 关机历史记录
    LogSocStrc Soc;
    
    u8 ProductActive;
    u8 DisUVLock;
    u8 ChgOVLock;
    u8 ChgFLLock;
    u8 DisOTLock;
    u8 ChgOTLock;
    u8 CmosOTLock;
    u8 DmosOTLock;
    
	u32 Crc;
} StrDataFlashPowerDownSave;

#define SECOND_TO_HOUR				3600UL

#define USERLOG_100MS_TIME				(100/TIMEBASE_LOOP)    /* 用户LOG信息0.1秒更新计时 */


extern XRAM StrDataFlashAtOnceSave  DataFlashAtOnceSave;         //立即调用保存的数据
extern XRAM StrDataFlashPowerDownSave DataFlashPowerDownSave;    //掉电时保存的数据


#if defined ( __CC_ARM   )
#pragma pack()						// 取消上面的4字节对齐
#endif

#define VDD_CALIB_DATA			DataFlashAtOnceSave.VddData
#define VDD_VERIFY_DATA			DataFlashAtOnceSave.VddVerify

#define DATA_DSG_UV_LOCK		DataFlashPowerDownSave.DisUVLock

#define CHG_OV_LOCK()			{do {DataFlashPowerDownSave.ChgOVLock = OV_LOCKED_MARK;} while(0);}
#define CHG_OV_UNLOCK()			{do {DataFlashPowerDownSave.ChgOVLock = 0;} while(0);}
#define IS_CHG_OV_LOCKED()		(DataFlashPowerDownSave.ChgOVLock == OV_LOCKED_MARK)
#define CHG_FL_LOCK()			{do {DataFlashPowerDownSave.ChgFLLock = FL_LOCKED_MARK;} while(0);}
#define CHG_FL_UNLOCK()			{do {DataFlashPowerDownSave.ChgFLLock = 0;} while(0);}
#define IS_CHG_FL_LOCKED()		(DataFlashPowerDownSave.ChgFLLock == FL_LOCKED_MARK)

#define DSG_OT_LOCK()			{do {DataFlashPowerDownSave.DisOTLock = OT_LOCKED_MARK;} while(0);}
#define DSG_OT_UNLOCK()			{do {DataFlashPowerDownSave.DisOTLock = 0;} while(0);}
#define IS_DSG_OT_LOCKED()		(DataFlashPowerDownSave.DisOTLock == OT_LOCKED_MARK)

#define CHG_OT_LOCK()			{do {DataFlashPowerDownSave.ChgOTLock = OT_LOCKED_MARK;} while(0);}
#define CHG_OT_UNLOCK()			{do {DataFlashPowerDownSave.ChgOTLock = 0;} while(0);}
#define IS_CHG_OT_LOCKED()		(DataFlashPowerDownSave.ChgOTLock == OT_LOCKED_MARK)

#define DMOS_OT_LOCK()			{DataFlashPowerDownSave.DmosOTLock = OT_LOCKED_MARK;}
#define DMOS_OT_UNLOCK()		{DataFlashPowerDownSave.DmosOTLock = 0;}
#define IS_DMOS_OT_LOCKED()		(DataFlashPowerDownSave.DmosOTLock == OT_LOCKED_MARK)
#define CMOS_OT_LOCK()			{DataFlashPowerDownSave.CmosOTLock = OT_LOCKED_MARK;}
#define CMOS_OT_UNLOCK()		{DataFlashPowerDownSave.CmosOTLock = 0;}
#define IS_CMOS_OT_LOCKED()		(DataFlashPowerDownSave.CmosOTLock == OT_LOCKED_MARK)

#define PRODUCT_ACTIVE()		{do {DataFlashPowerDownSave.ProductActive = PRODUCT_ACTIVED;} while(0);}
#define PRODUCT_UNACTIVE()		{do {DataFlashPowerDownSave.ProductActive = 0;} while(0);}
#define IS_PRODUCT_ACTIVED()	(DataFlashPowerDownSave.ProductActive == PRODUCT_ACTIVED)


#ifdef AFE_MODEULE
#define DATA_AFE_MC_BIAS		(DataFlashAtOnceSave.AfeMCBias)
#define DATA_AFE_SC_BIAS		(DataFlashAtOnceSave.AfeSCBias)
#define DATA_CUR_K_CHG			DataFlashAtOnceSave.Current_K[K_CHG_CUR]
#define DATA_CUR_K_DSGM			DataFlashAtOnceSave.Current_K[K_DSGM_CUR]
#endif
#define DATA_VOL_K_PACK     DataFlashAtOnceSave.VPackKi

void AtOnceSaveDataWrite(void);
void AtOnceSaveDataRead(void);
void PowerDownSaveDataWrite(void);
void PowerDownSaveDataRead(void);
void DataRecInit(void);
void UserLogInit(void);
void DataLogPwrOffSave(u8 Flag);
void DataLogPwrOnRecord(void);

#endif
