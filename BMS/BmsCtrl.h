/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : BmsCtrl.h
  版 本 号   : 初稿
  作    者   : 
  生成日期   : 2019年5月25日 星期六
  最近修改   :
  功能描述   : BMS模式检测、执行H头文件
  函数列表   :
  修改历史   :

******************************************************************************/

#ifndef __BMS_CTRL_H__
#define __BMS_CTRL_H__

MainDef u8 TimerBmsFlag;

typedef enum
{
	BMS_WORK_IDLE,      						// 空闲模式
	BMS_WORK_DSG,             					// 放电模式
	BMS_WORK_CHG,             					// 充电模式
	BMS_WORK_SLEEP,           					// 休眠模式
	BMS_WORK_SHUTDOWN,        					// 掉电模式
	BMS_WORK_NUM
}_BMS_WORK_MODE_T;

typedef enum
{
	SYSTEM_INVALID = 0,      					// 禁用模式
	SYSTEM_WORK = 1,             				// 工作模式
	SYSTEM_UPDATING = 2,             			// 升级模式
	SYSTEM_FAULT = 3,           				// 故障模式
	SYSTEM_CHARGING = 4,        				// 充电中
	SYSTEM_DISCHARGING = 5,						// 放电中
	SYSTEM_CHARGE_FAIL = 6,						// 充电故障
	SYSTEM_DISCHARGE_FAIL = 7,					// 放电故障
	SYSTEM_FULL_CHARGE = 8,						// 满充
	SYSTEM_CHARGE_IN = 9,						// 充电器接入
	SYSTEM_SLEEP = 10,              //休眠模式
}_SYS_STATE_T;

typedef enum{
	LOG_NONE = 0,
    LOG_CELL_OV = 0x01,       		// +1  单体过压 0x01
    LOG_CELL_UV,              		// +2  单体欠压 0x02
    LOG_CELL_ERR,         			// +3  电芯采样断线 0x03
    LOG_DISG_OC,      				// +4  放电过流 0x04
    LOG_CHG_OC,      				// +5  充电过流 0x05
    LOG_DISG_SC,             		// +6  短路 0x06
    LOG_DISG_OT, 					// +7  放电过温 0x07
    LOG_CHG_OT,         			// +8  充电过温 0x08
    LOG_CHG_MOT,          			// +9  MOS过温 0x09 -->Mos充电过温
    LOG_DISG_MOT,       			// +10 MOS过温 0x0A -->Mos放电过温
    LOG_Afe_ERR,					// +11 AFE通讯故障 0x0B
	LOG_CAN_ERR,             		// +12 CAN通讯故障 0x0C
	LOG_RTC_ERR,          			// +13 RTC通讯故障 0x0D
 	LOG_EXFLASH_ERR,         		// +14 外部flash故障 0x0E
	LOG_CELL_NTC_ERR,           	// +15 电芯采样NTC开路 0x0F
	LOG_MOS_NTC_ERR,           		// +16 MOS采样NTC开路 0x10
	LOG_FUSE_BURN,          		// +17 FUSE熔断 0x11
    LOG_CHG_ERR,         			// +18 充电器不匹配 0x12
    LOG_MUTI_ERR,          			// +19 并包故障 0x13
		LOG_CMOS_ERR,               // +20 充电MOS故障 0x14
		LOG_DMOS_ERR,               // +21 放电MOS故障 0x15
}BMS_LOG_EVENT;

extern BMS_LOG_EVENT BmsGetWarnState(void);
extern _SYS_STATE_T BmsGetSystemState(void);
extern void BctTimerCallBack(u8 ticks);

extern void BmsCtrl(void);
extern _BMS_WORK_MODE_T BmsGetWorkMode(void);


#endif
