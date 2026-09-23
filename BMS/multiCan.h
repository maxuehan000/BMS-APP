/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : mutican.c
  版 本 号   : 初稿
  作    者   : William
  生成日期   : 2025年1月6日 
  最近修改   :
  功能描述   : 多包并联.h文件
  函数列表   :
  修改历史   :
  1.日    期   : 2025年1月6日  
    作    者   : William
    修改内容   : 创建文件

******************************************************************************/

#ifndef __MULTICAN_H__
#define __MULTICAN_H__
#define MULTIPLE_PARALLEL   /*打开则进行并包*/
#if 1
//#define ADCOMPUTERIR        /*打开是AD采样来计算总内阻 */
#ifndef ADCOMPUTERIR
#define AFECOMPUTERIR       /*打开是afe采样来计算单节内阻*/
#endif
#endif
#ifdef ADCOMPUTERIR
#define INTERNAL_RES      55  /*电池包总内阻 =*2毫欧  55表示110毫欧*/
#define INTERNAL_RES_MAX  247
#define INTERNAL_RES_MIN  26
#define IRCHANGE          1
#endif
#ifdef AFECOMPUTERIR 
#define INTERNAL_RES      40  /*电池包单内阻 =*0.2毫欧 40表示8毫欧  */
#define INTERNAL_RES_MAX  190
#define INTERNAL_RES_MIN  20
#define IRCHANGE          1
#endif
#define ICHANGEMIN        200 /*计算前的电流变化<0.1A*/
#define IRCOMPUTERVMIN    3300
#define IRCOMPUTERVMAX    4050
#define IRCOMPUTERIMIN    1800
typedef struct
{
    u8 DisMosAllow   :1;/*根据并联状态进行放电mos开关允许*/
    u8 ChgMosAllow   :1;/*根据并联状态进行充电mos开关允许*/ 
    
    u8 QUICKDisMos   :1;/*根据并联状态进行快速放电mos开允许*/
    u8 QUICKChgMos   :1;/*根据并联状态进行快速充电mos开允许*/  
    
    u8 CanTestMark   :1;/*can测试指令 =1 允许主动开mos，倒计时1s清零*/
}MtBmsAllowStr;
typedef union
{
    u8            byte;
    MtBmsAllowStr bit;
}MtBmsAllowUnion;
MainDef u16 computeriRAVE;
MainDef MtBmsAllowUnion MtBmsAllow; 
u8 MTCanReceiveProcess(CanRxMessage RxMsg, CanTxMessage *TxMsg);
extern void MTCanCtrl(void); 
extern u8 MultiCanAddr(void);
/*多包并联逻辑下需要调用如下代码，有mos控制变化，则需要尽快更新状态*/
extern void fetchangeflushstatus(u8 fet);
extern void HwCompInit(void);

extern u8 ReadMtOutputState(void);
extern u8 ReadMtChgPluginState(void);
/*当前单包且休眠前dmos处于开启,则自主唤醒不识别canerr*/
extern u8 SingleAndMosOnCheck(void);
extern void SetSingleMosOnState(void);
extern void ClearSingleMosOnState(void);
#endif


