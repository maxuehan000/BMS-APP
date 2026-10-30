/******************************************************************************

                  版权所有 (C), 2001-2019, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : soc.h
  版 本 号   : 初稿
  作    者   : hbquan
  生成日期   : 2019年4月17日 星期三
  最近修改   :
  功能描述   : SOC模块头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年4月17日 星期三
    作    者   : hbquan
    修改内容   : 创建文件

******************************************************************************/

#ifndef __SOC_H__
#define __SOC_H__

#include "McuHal.h"
#include "SysDefs.h"
#include "User.h"


#ifdef SOC_SUPPORT

#define SOC_OCV_VERIFY              0xAA                    /* SOC是否OCV校验锁定值 */

#define SOC_1MAH_SMOOTH
//#define USE_FROZEN_CAPACITY
#define BAT_PARAL_NUM				CELL_PARAL

#if (BAT_TYPE == BAT_SDI_35E)
#define CAPACITY_DESIGN             (3350*BAT_PARAL_NUM)    /* 电芯设定容量，单位: mAh */
#define CAPACITY_TYPE				(3200*BAT_PARAL_NUM)
#elif (BAT_TYPE==BAT_SDI_50E)
#define CAPACITY_DESIGN             (5000*BAT_PARAL_NUM)    /* 电芯设定容量，单位: mAh */
#define CAPACITY_TYPE				(4600*BAT_PARAL_NUM)
#elif (BAT_TYPE==BAT_DCM_26E)
#define CAPACITY_DESIGN             (2600*BAT_PARAL_NUM)    /* 电芯设定容量，单位: mAh */
#define CAPACITY_TYPE				(2500*BAT_PARAL_NUM)
#elif (BAT_TYPE==BAT_SDI_50P)
#define CAPACITY_DESIGN             (5000*BAT_PARAL_NUM)    /* 电芯设定容量，单位: mAh */
#define CAPACITY_TYPE				(4750*BAT_PARAL_NUM)
#elif (BAT_TYPE==BAT_EVE_33V)
#define CAPACITY_DESIGN             (3200*BAT_PARAL_NUM)    /* 电芯设定容量，单位: mAh */
#define CAPACITY_TYPE				(3050*BAT_PARAL_NUM)
#endif

#define FCC_UPDATE_MAX              (u16)(CAPACITY_DESIGN*0.01)         /* 1%步进量 */
#define FCC_CYCLE_DECEND			(u16)(CAPACITY_TYPE*0.2/300)		/* 300个循环-20%衰减 */

#define CAPACITY_ERR_MIN            ((u16)(CAPACITY_DESIGN*0.7))    /* 最小有效容量值 */
#define CAPACITY_ERR_MAX            ((u16)(CAPACITY_DESIGN*1.2))    /* 最大有效容量值 */

#ifdef USE_FROZEN_CAPACITY
#define MAX_FROZEN_CAPACITY         CAPACITY_DESIGN/3
#define DEFAULT_FROZEN_CAPACITY     100
#define RMC_FCC_DELTA               500                             /* 20%偏差 */
#endif

#define SOC_INIT_DELAY              (50)                		/* 系统上电后需延时的时长，此后方可进行SOC检测 */

#define CYCLE_DSG_CAPACITY          ((u16)(CAPACITY_DESIGN*0.80))   /* 一次放电所对应的放电容量判定 */
#define MAX_CYCLE_COUNT             500                             /* 最大循环次数 */

#define MAMS_TO_MAH                 (3600000L)              	/* 1000ms*60*60 = 3600000 ->1mAH */
#define SOC_CUR_TIME_MAX            (400)       				/* SOC计算的最大时间间隔，单位: ms，BQ系列电流采样时间250ms */

#define SOC_CHG_CUR_MAX             (  40000L)                	/* SOC可检测最大充电电流 */
#define SOC_DIS_CUR_MAX             (-150000L)                   /* SOC可检测最大放电电流 */

#define OCV_INIT_SET		0
#define OCV_STATIC_SET		1

typedef struct
{
	u8  OcvFlag;
    u16 InitDelay;      /* SOC上电延时，单位:ms */
    u8  OvFlag;
    u8  FcFlag;
    u8  UvFlag;
    u8  OcvWait;
    u8  OcvReset;
} SOC_CTRL_s;

typedef struct
{
    u8  Soc;        /* OCV表: soc值 */
    u16 Vol;        /* OCV表: 开路电压值 */
} SOC_2_OCV;


MainDef u8  TimerSocFlag;           /* SOC基准计时器 */
MainDef u8  SocUpdateFlag;          /* SOC更新标识 */
MainDef u16 SocDeltaTime;           /* SOC库仑积分计算时对应的:ΔTime */
MainDef u16 SocBaseTimer;           /* SOC时间计时器 */
//MainDef u32 SocImAms;             /* mA*ms, mAH积分余数，换算参见MAMS_TO_MAH定义 */
MainDef u32 Dynamic1mAh;
MainDef SOC_CTRL_s SocCtr;

#define SocInfo			DataFlashPowerDownSave.Soc
#define SOC_DEADDISCUR              (-50)
#define SOC_DEADCHGCUR              (50)
#define SOC_STATICCUR               (-10)

extern void SocInit(void);
extern void SocAlertInt(void);
extern void SocTimeUpdate(u8 TimeOut);
extern void SocCtrl(void);
extern void SocDispCtrl(void);
extern u8 SocOcvSet(u8 set);
extern void SocUpdateByCap(u16 mAh);

extern u8 SocIsOcvWait(void);
extern void SocOcvTodo(void);
#endif  /* SOC_SUPPORT */

#endif  /*__SOC_H__ */
