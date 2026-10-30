/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : BatterCheck.h
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 电芯、电量检测H头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BATTERCHECK__H
#define __BATTERCHECK__H


extern void BatCheck(void);
extern void BatCellVolUpdate(void);
//extern u8 BatVddVerify(void);
//extern void BatVddCalib(void);
//extern u16 BatReadVref2Vdd(void);
extern u16 AdcNtcToTempK(u8 NtcType, u16 AdcNtc);
extern s32 BatOpaCurGet(void);
extern void BatOpaCurUpdate(void);
	
#endif
