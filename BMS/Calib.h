/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : Charge.h
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 充电检测、执行H头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/

#ifndef __CALIB_H__
#define __CALIB_H__

#include "McuHal.h"
#include "SysDefs.h"
#include "User.h"

#define CUR_K_DEFAULT  		10000
#define CUR_K_MAX   		12000
#define CUR_K_MIN   		8000
#define CUR_ZERO_MAX 		500
#define CUR_ZERO_DEFAULT  	0

#define CELL_VOL_K_MIN 		9000
#define CELL_VOL_K_MAX 		11000
#define CELL_VOL_K_DEFAULT 	11000

#define VOL_K_MIN			8000
#define VOL_K_MAX			12000
#define VOL_K_DEFAULT		10000

MainDef u8 CalibFlag;

extern u8 CalibPackVol(u8 *uartbuf);
extern void CalibZeroCurRun(s32 cur);                //在采样电流处调用，用于0电流求平均
extern void CalibZeroCurrent(void); 
extern void CalibDsgCur(u8 *uartbuf);
extern void CalibChgCur(u8 *uartbuf);
extern void CalibBat(void);
extern void CalibInit(void);

//extern void CalibChargerVol(u8 *uartbuf); 			// 充电器总电压校准
#endif
