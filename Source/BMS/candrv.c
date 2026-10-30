/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : candrv.c
  版 本 号   : 初稿
  作    者   : William
  生成日期   : 2025年1月6日 
  最近修改   :
  功能描述   : can通讯底层C源文件
  函数列表   :
  修改历史   :
  1.日    期   : 2025年1月6日  
    作    者   : William
    修改内容   : 创建文件

******************************************************************************/


#include "board.h"

static u8 TimerCan1msFlag,TimerCan10msFlag;

/* CAN2 包间通信波特率参数(1000K, 36MHz时钟: 36M/(2*18)=1M, 采样点77.8%) */
#define  MT_CAN_RSJW               CAN_RSJW_1tq
#define  MT_CAN_TBS1               CAN_TBS1_13tq
#define  MT_CAN_TBS2               CAN_TBS2_4tq
#define  MT_CAN_PRESCALER          2

/* CAN2 包间通信收发队列 */
queue MtRxQueue = queue(CanRxMessage, 32);
queue MtTxQueue = queue(CanTxMessage, 16);

/*************** CAN2 包间通信驱动 ****************/

/**
 * @brief CAN2 过滤器初始化：接收并包广播 0x1A0~0x1AF + 测试指令 0x2A0
 */
void MtCanFilterInit(void)
{
    CAN_FilterInitType CAN_FilterInitStructure;
    u32 filter_std;
    u32 filter_mask;

    CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
    CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.Filter_Act            = ENABLE;

    /* Filter 0: 并包广播 0x1A0~0x1AF (掩码 0x7F0) */
    CAN_FilterInitStructure.Filter_Num            = 0;
    filter_std  = (0x1A0UL << 21);
    filter_mask = (0x7F0UL << 21);
    CAN_FilterInitStructure.Filter_HighId         = (u16)(filter_std >> 16);
    CAN_FilterInitStructure.Filter_LowId          = (u16)(filter_std);
    CAN_FilterInitStructure.FilterMask_HighId     = (u16)(filter_mask >> 16);
    CAN_FilterInitStructure.FilterMask_LowId      = (u16)(filter_mask);
    CAN2_InitFilter(&CAN_FilterInitStructure);

    /* Filter 1: 测试指令 0x2A0 (精确匹配) */
    CAN_FilterInitStructure.Filter_Num            = 1;
    filter_std  = (0x2A0UL << 21);
    filter_mask = (0x7FFUL << 21);
    CAN_FilterInitStructure.Filter_HighId         = (u16)(filter_std >> 16);
    CAN_FilterInitStructure.Filter_LowId          = (u16)(filter_std);
    CAN_FilterInitStructure.FilterMask_HighId     = (u16)(filter_mask >> 16);
    CAN_FilterInitStructure.FilterMask_LowId      = (u16)(filter_mask);
    CAN2_InitFilter(&CAN_FilterInitStructure);

    CAN_INTConfig(CAN2, CAN_INT_FMP0, ENABLE);
}

/**
 * @brief CAN2 外设初始化 (PB12/PB13, 1000K)
 */
void MtHwCanInit(void)
{
    CAN_InitType CAN_InitStructure;

    CAN_InitStruct(&CAN_InitStructure);
    CAN_DeInit(CAN2);

    CAN_InitStructure.TTCM              = DISABLE;
    CAN_InitStructure.ABOM              = ENABLE;
    CAN_InitStructure.AWKUM             = DISABLE;
    CAN_InitStructure.NART              = DISABLE;
    CAN_InitStructure.RFLM              = DISABLE;
    CAN_InitStructure.TXFP              = ENABLE;
    CAN_InitStructure.OperatingMode     = CAN_Normal_Mode;
    CAN_InitStructure.RSJW              = MT_CAN_RSJW;
    CAN_InitStructure.TBS1              = MT_CAN_TBS1;
    CAN_InitStructure.TBS2              = MT_CAN_TBS2;
    CAN_InitStructure.BaudRatePrescaler = MT_CAN_PRESCALER;
    CAN_Init(CAN2, &CAN_InitStructure);

    MtCanFilterInit();
}

/**
 * @brief CAN2 接收处理：从接收队列取一帧分发到并包协议
 */
void MtCanReceiveProcess(void)
{
    CanRxMessage RxMsg;

    if (0 == queue_pop_front(MtRxQueue, &RxMsg))
    {
        return;
    }
    MTCanReceiveProcess(RxMsg, 0);
}

/**
 * @brief CAN2 发送处理：从发送队列取一帧发送
 */
void MtCanTransmitProcess(void)
{
    CanTxMessage QueueMsg;

    if (0 == queue_pop_front(MtTxQueue, &QueueMsg))
    {
        return;
    }
    if (BatStatus.Bits.PowerOff == 0)
    {
        if (CAN_TxSTS_NoMailBox == CAN_TransmitMessage(CAN2, &QueueMsg))
        {
            queue_push_front(MtTxQueue, &QueueMsg);
        }
    }
}

void CanTimerCallBack(u8 ticks)
{
	static u8 Div10ms = 0;
	
	TimerCan1msFlag ++;

	Div10ms += ticks;
	if (Div10ms < 10)
		return;
	Div10ms -= 10; 
    TimerCan10msFlag=1;
}


void CanInit(void)
{
	MtHwCanInit();
}

void CanCtrl(void)
{
    if(TimerCan10msFlag)
    {
        TimerCan10msFlag = 0;

		if(BatStatus.Bits.PowerOff == 1) //关机不处理发送CAN数据
			return;
				
        MtCanReceiveProcess();
        MTCanCtrl();
        MtCanTransmitProcess();
    }
}
