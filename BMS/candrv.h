/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : candrv.c
  版 本 号   : 初稿
  作    者   : William
  生成日期   : 2025年1月6日 
  最近修改   :
  功能描述   : can驱动.h文件
  函数列表   :
  修改历史   :
  1.日    期   : 2025年1月6日  
    作    者   : William
    修改内容   : 创建文件

******************************************************************************/


#ifndef __CAN_DRV_H
#define __CAN_DRV_H


#if 0
/**************************hal layer**********************/
#define SW_CAN_DEV				CAN
/* CAN波特率参数配置 */
#define  CAN_BAUDRATE_1M            1
#define  CAN_BAUDRATE_500K          2
#define  CAN_BAUDRATE_250K          3
#define  CAN_BAUDRATE_125K          4
#define  CAN_BAUDRATE               CAN_BAUDRATE_250K

#if(CAN_BAUDRATE==CAN_BAUDRATE_1M)
#define   CAN_BIT_RSJW              CAN_RSJW_1tq
#define   CAN_BIT_BS1               CAN_TBS1_5tq
#define   CAN_BIT_BS2               CAN_TBS2_2tq
#define   CAN_BAUDRATEPRESCALER     2 
#elif(CAN_BAUDRATE==CAN_BAUDRATE_500K)
#define   CAN_BIT_RSJW              CAN_RSJW_1tq
#define   CAN_BIT_BS1               CAN_TBS1_10tq
#define   CAN_BIT_BS2               CAN_TBS2_5tq
#define   CAN_BAUDRATEPRESCALER     2
#elif(CAN_BAUDRATE==CAN_BAUDRATE_250K)
#define   CAN_BIT_RSJW              CAN_RSJW_1tq
#define   CAN_BIT_BS1               CAN_TBS1_13tq
#define   CAN_BIT_BS2               CAN_TBS2_2tq
#define   CAN_BAUDRATEPRESCALER     4 
#elif(CAN_BAUDRATE==CAN_BAUDRATE_125K)
#define   CAN_BIT_RSJW              CAN_RSJW_1tq
#define   CAN_BIT_BS1               CAN_TBS1_10tq
#define   CAN_BIT_BS2               CAN_TBS2_5tq
#define   CAN_BAUDRATEPRESCALER     8
#endif 

//extern queue SwTxQueue;

#endif 

extern void CanTimerCallBack(u8 ticks);  
extern void CanInit(void);
extern void CanCtrl(void);
#endif


