/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : key.h
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2017年1月6日 星期五
  最近修改   :
  功能描述   : 按键识别H头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2017年1月6日 星期五
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/
#ifndef __KEY__H__
#define __KEY__H__


typedef enum
{
	KEY_NONE,		// 0	no
	KEY_MILD,		// 1	轻触 < 3S
	KEY_SHORT,		// 2	短按 3~10S
	KEY_LONG,		// 3	长按 >10S
	KEY_PRESS,		// 4	按下
	KEY_RELEASE,	// 5弹起
  KEY_ON,
  KEY_OFF,
	KEY_NUM
} _KEY_PADS_T;
typedef struct {
  unsigned char showType;     //显示状态
  unsigned short showTimeCnt; //显示时间计数
}LedShowCtrler;

typedef enum
{
  LedNoShow =0,     //无显示
  LedShowSoc,       //显示SOC
  LedShowSoh,       //显示SOh
  LedShowChg,       //显示充电
  LedShowDis,       //显示放电
  LedShowFault,       //显示故障
}LED_SHOW;
extern LedShowCtrler     ledShow;
void KeyInit(void);
void KeyCheck(void);
void KeyLedReset(void);

void KeyOnReset(void);
extern u8  GetDetId(void);
#endif
