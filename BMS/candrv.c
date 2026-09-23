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


/***************define Tx Rx Message Queue****************/
#if 0

//queue SwRxQueue = queue(CanRxMessage, 32);
//queue SwTxQueue = queue(CanTxMessage, 16);
static CanRxMessage SwCanRxMsgNow;

void CAN_Filter_Init(void)
{
    CAN_FilterInitType CAN_FilterInitStructure;

#if 0
    u32 data;
    /* CAN filter init */  
	/*       5  | 5   3    |  8 | 8       3
		    SID  DID CMD     FUN  SFUN   IDE=1|RTR=0|0
		SID:04/05/06/07  1E
		DID:05/06        1F
	*/
    CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
    CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
    CAN_FilterInitStructure.Filter_Act            = ENABLE;
    
    CAN_FilterInitStructure.Filter_Num            = 0;
    data = (ID_BMS_NODE<<(8+8+3+3))|0x0004U;
    CAN_FilterInitStructure.Filter_HighId         = (data>>16);			// DID=0x05(MASTER)
    CAN_FilterInitStructure.Filter_LowId          = (data>>0);			// IDE=1|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0x07C0;
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0007;    
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;    
    CAN_InitFilter(&CAN_FilterInitStructure);    

    CAN_FilterInitStructure.Filter_Num            = 1;
    CAN_FilterInitStructure.Filter_HighId         = 0x07C0;				// DID=0x1F
    CAN_FilterInitStructure.Filter_LowId          = 0x0004;				// IDE=1|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0x07C0;
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0007;    
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO1;    
    CAN_InitFilter(&CAN_FilterInitStructure);

    CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdListMode;
    data = (0x04028000<<3)|0x04;	// UPDATA_DATA_EID      0x04028000
    CAN_FilterInitStructure.Filter_Num            = 2;
    CAN_FilterInitStructure.Filter_HighId         = data>>16;			
    CAN_FilterInitStructure.Filter_LowId          = data;				// IDE=1|RTR=0|0 

    data = (0x05020000<<3)|0x04;	// UPDATA_DATA_EID      0x05020000
    CAN_FilterInitStructure.FilterMask_HighId     = data>>16;
    CAN_FilterInitStructure.FilterMask_LowId      = data;    
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO1;    
    CAN_InitFilter(&CAN_FilterInitStructure);

    data = (BW_ID_EXT_PC<<3)|0x04;	// BW protocol
    CAN_FilterInitStructure.Filter_Num            = 3;
    CAN_FilterInitStructure.FilterMask_HighId     = data>>16;
    CAN_FilterInitStructure.FilterMask_LowId      = data;    
#ifdef BMS_BP_SLAVER
	data = (AND_EXCHANG_ID<<3)|0x04;// AND protocol exchange ID
    CAN_FilterInitStructure.Filter_HighId         = data>>16;			
    CAN_FilterInitStructure.Filter_LowId          = data;
#endif
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;    
    CAN_InitFilter(&CAN_FilterInitStructure);
    
    /* IT Configuration for CAN */
    CAN_INTConfig(SW_CAN_DEV, CAN_INT_FMP0, ENABLE);
    CAN_INTConfig(SW_CAN_DEV, CAN_INT_FMP1, ENABLE);

#else    
    /* 不过滤ID的设置 */ 
	CAN_FilterInitStructure.Filter_Num            = 0;
	CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
	CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
	CAN_FilterInitStructure.Filter_HighId         = 0;
	CAN_FilterInitStructure.Filter_LowId          = 0;
	CAN_FilterInitStructure.FilterMask_HighId     = 0;
	CAN_FilterInitStructure.FilterMask_LowId      = 0;
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.Filter_Act            = ENABLE;
	CAN_InitFilter(&CAN_FilterInitStructure); 
	CAN_FilterInitStructure.Filter_Num            = 1;
	CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
	CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
	CAN_FilterInitStructure.Filter_HighId         = 0;
	CAN_FilterInitStructure.Filter_LowId          = 0;
	CAN_FilterInitStructure.FilterMask_HighId     = 0;
	CAN_FilterInitStructure.FilterMask_LowId      = 0;
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO1;
	CAN_FilterInitStructure.Filter_Act            = ENABLE;
	CAN_InitFilter(&CAN_FilterInitStructure); 
	CAN_INTConfig(SW_CAN_DEV, CAN_INT_FMP0, ENABLE);
	CAN_INTConfig(SW_CAN_DEV, CAN_INT_FMP1, ENABLE);
#endif
}

#endif

#if 0
/**
 * @brief  CAN Interrupt Configures .
 */
void CAN_NVIC_Config(void)
{
    NVIC_InitType NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = CAN_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = CAN_RX1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void CAN_RX0_IRQHandler(void)
{  
	CanRxMessage RxMsg;

	CAN_ReceiveMessage(SW_CAN_DEV, CAN_FIFO0, &RxMsg);
	queue_push_back(SwRxQueue, &RxMsg);
}
void CAN_RX1_IRQHandler(void)
{   
	CanRxMessage RxMsg;

	CAN_ReceiveMessage(SW_CAN_DEV, CAN_FIFO1, &RxMsg);
	queue_push_back(SwRxQueue, &RxMsg);   
}
#endif

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

#if 0
void HwCanInit(void)
{
    CAN_InitType CAN_InitStructure;
     /* Struct init*/
    CAN_InitStruct(&CAN_InitStructure);
    
    /* enable CAN clk */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_CAN, ENABLE);
    
    /* CAN register deinit */
    CAN_DeInit(SW_CAN_DEV);  
    
    /* CAN cell init */
    CAN_InitStructure.TTCM              = DISABLE;
    CAN_InitStructure.ABOM              = ENABLE;
    CAN_InitStructure.AWKUM             = DISABLE;
    CAN_InitStructure.NART              = DISABLE;		// DISABLE
    CAN_InitStructure.RFLM              = DISABLE;
    CAN_InitStructure.TXFP              = ENABLE;
    CAN_InitStructure.OperatingMode     = CAN_Normal_Mode;
#ifdef CAN_LOOP
    CAN_InitStructure.OperatingMode     = CAN_LoopBack_Mode;
#endif
    CAN_InitStructure.RSJW              = CAN_BIT_RSJW;
    CAN_InitStructure.TBS1              = CAN_BIT_BS1;
    CAN_InitStructure.TBS2              = CAN_BIT_BS2;
    CAN_InitStructure.BaudRatePrescaler = CAN_BAUDRATEPRESCALER;
    /*Initializes the CAN */
    CAN_Init(SW_CAN_DEV, &CAN_InitStructure);
    
    CAN_Filter_Init();
    
//    CAN_NVIC_Config(); 
}


void CanTransmitProcess(void)
{ 
	CanTxMessage QueueMsg;
	// need test
	if (CAN_TxSTS_NoMailBox == CAN_TransmitState(SW_CAN_DEV))
	{
		return;
	}

	// pop msg from queue
	if (0 == queue_pop_front(SwTxQueue, &QueueMsg))
	{
		return;			// no data in queue
	}
    if(BatStatus.Bits.PowerOff==0)
	CAN_TransmitMessage(SW_CAN_DEV, &QueueMsg);
}


void CanInit(void)
{
	HwCanInit();  
}
#endif
 
void CanCtrl(void)
{
    if(TimerCan10msFlag)
    {
        TimerCan10msFlag = 0;
			
		if(BatStatus.Bits.PowerOff == 1) //关机不处理发送CAN数据
			return;
				
        MTCanCtrl();
    }
}


