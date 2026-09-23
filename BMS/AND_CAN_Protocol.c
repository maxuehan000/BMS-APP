
#include "board.h" 


#define BMS_BP_MASTER

#ifdef CAN_AND_PROTOCOL

//#define AND_OTA_FUNC

#define BW_ID_SWITCH					0x00000001UL

#define BW_ID_STD_PC					0x07F1U
#define BW_ID_STD_BMS					0x07F2U

#define SL_CAN_ECU_ID1 					0x1B0		/*参见通讯协议定义*/ 
#define SL_CAN_ECU_ID2 					0x1C0		/*参见通讯协议定义*/ 
#define SL_CAN_ECU_ID3 					0x1E0		/*参见通讯协议定义*/ 

static u8 Ecu_ChgPlugin = 0x5A;		/*有ECU检测到充电器在*/
static u8 Ecu_Sleep = 0x5A;  /*ECU控制休眠*/
static u8 Ecu_LockPower = 0x5A;  /*ECU锁定电源输出*/
/***************define Tx Rx Message Queue****************/
queue AndRxQueue = queue(CanRxMessage, 32);
queue AndTxQueue = queue(CanTxMessage, 16);

#pragma pack(1)						// 按1字节对齐
typedef struct
{
	u8 SA 		: 8;
	u8 DA		: 8;
	u8 PF		: 8;
	u8 DP		: 1;
	u8 Res		: 1;
	u8 Priority	: 3;
	u8 Unused	: 3;
}And_Pdu1_ID;
typedef struct
{
	u8 SA 		: 8;
	u16 PS		: 16;
	u8 DP		: 1;
	u8 Res		: 1;
	u8 Priority	: 3;
	u8 Unused	: 3;
}And_Pdu2_ID;

typedef union
{
	u32 		ExtID;
	And_Pdu1_ID	Pdu1ID;
	And_Pdu2_ID	Pdu2ID;
}And_Can_ID;
#pragma pack()						// 取消1字节对齐

typedef enum
{
//	ID_NULL = 0x00,

	ID_DSP1 = 0x28,
	ID_DSP2 = 0x29,
	ID_BMS_SL = 0xF4,
	ID_MCU1 = 0xEF,
	ID_MCU2 = 0xF0,
	ID_SW = 0x05,
	ID_TD = 0xF9,
	ID_CHG = 0x56, //充电器
	ID_IOT = 0x18,
	ID_SHIFTER = 0x20,
	ID_SHIFTER_KEY = 0x21,
	ID_BC = 0xFF, //广播
}ID_NODE;

#ifdef BMS_BP_MASTER
#define ID_BMS_NODE		ID_BMS_SL
#endif

#define PF_MAX_FOR_PDU1				239
#define PRIORITY_INFO_DEFAULT		6
#define BMS_CAN_BC_OUT_TIME			(3000/TIMEBASE_LOOP)	// 3S, BMS stop broadcase




#define CAN_TIME_OUT_DELAY			(3000)					// 4500*1mS = 4.5s

typedef enum
{
	STD_BMS_INFO1 = 0x1B01,
	STD_BMS_INFO2 = 0x1B02,
	STD_BMS_INFO3 = 0x1B03,
    STD_BMS_INFO4 = 0x1B04,

	STD_BMS_ACK01 = 0x1E01,
	STD_BMS_ACK02 = 0x1E02,
	STD_BMS_ACK03 = 0x1E03,
	STD_BMS_ACK04 = 0x1E04,
	STD_BMS_ACK05 = 0x1E05,
	STD_BMS_ACK06 = 0x1E06,
	STD_BMS_ACK07 = 0x1E07,
	STD_BMS_ACK08 = 0x1E08,
	STD_BMS_ACK09 = 0x1E09,
	STD_BMS_ACK10 = 0x1E0A,
	STD_BMS_ACK11 = 0x1E0B,
	STD_BMS_ACK12 = 0x1E0C,
	STD_BMS_ACK13 = 0x1E0D,
}BMS_STD_E;

void SL_CanBmsMsgTransmit(BMS_STD_E index);

static u16 CanToTimer = 0;
//static u16 BmsLogIndex = 0;

typedef enum
{
	//BMS->其他
	PGN_BMS_INFO1 = 0x01FF,                 //电池包信息(100ms广播）
	PGN_BMS_INFO2 = 0x02FF, 	        	//电池包信息(100ms广播）

	PGN_BMS_FW_INFO = 0x5000,         		//固件信息
	PGN_BMS_CAP_INFO = 0x5100,         		//容量信息
	PGN_BMS_STATE_INFO = 0x5200,			//状态信息

}BMS_PGN_E;




static CanRxMessage AndCanRxMsgNow;
static CanRxMessage SlCanRxMsgNow;

typedef struct
{
	u32 Pgn;
	u8  TotalPack;
	u8  CurrentPackIndex;
	u16 PackBytes;
	u32 PackNum;
}CAN_Multi_Data;

typedef struct
{
	volatile u8 TxPeriodTimer[6];
	u16 BroadCaseTimer;
	u16 HeartBeatTimer;
	u8  WarnCode;
	u8  ProtCode;
	u8  ErroCode;
	u8  RxFlag;
	u16 RxOutTimer;
	CAN_Multi_Data  MultiData;
	CAN_Multi_Data  OtaMultiData;
}And_CAN_Info;
And_CAN_Info AndCanInfo = {0};

/**************************hal layer**********************/
#define AND_CAN_DEV				CAN
/* CAN波特率参数配置 */
#define  CAN_BAUDRATE_1M            1
#define  CAN_BAUDRATE_500K          2
#define  CAN_BAUDRATE_250K          3
#define  CAN_BAUDRATE_125K          4
#define  CAN_BAUDRATE               CAN_BAUDRATE_1M   //波特率1M

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
void CAN_Filter_Init(void)
{
    CAN_FilterInitType CAN_FilterInitStructure;
    u32 data;
	u32 filter_std;
#if 1
    /* CAN filter init */
		/*       3    1    1     |   8  |  8  |  8  |     3
				Pri  Res DataPage   PF     DA    SA    IDE=1|RTR=0|0
					-> 0x18
		*/
    CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
    CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
    CAN_FilterInitStructure.Filter_Act            = ENABLE;

	//点对点发送给电池包的消息
	CAN_FilterInitStructure.Filter_Num            = 0;
    data = (ID_BMS_NODE<<(8+3))|0x0004U;
    CAN_FilterInitStructure.Filter_HighId         = (data>>16);			// DA=0xF4(ID_BMS1)
    CAN_FilterInitStructure.Filter_LowId          = (data>>0);			// IDE=1|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0x0007;
    CAN_FilterInitStructure.FilterMask_LowId      = 0xF807;
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
		
	//多包数据
	// PGN REQ EA: 0xEA+BMSID+
	data = ((0xEA0000|((u32)ID_BMS_NODE<<8))<<3)|0x0004;
    CAN_FilterInitStructure.Filter_Num            = 1;
    CAN_FilterInitStructure.Filter_HighId         = (u16)(data>>16);
    CAN_FilterInitStructure.Filter_LowId          = (u16)(data>>0);
    data = (0xFFFF00<<3)|0x0007;
    CAN_FilterInitStructure.FilterMask_HighId     = (u16)(data>>16);
    CAN_FilterInitStructure.FilterMask_LowId      = (u16)(data>>0);
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
		
    // PGN REQ EC: 0xEC+BMSID+
    data = ((0xEC0000|((u32)ID_BMS_NODE<<8))<<3)|0x0004;
    CAN_FilterInitStructure.Filter_Num            = 2;
    CAN_FilterInitStructure.Filter_HighId         = (u16)(data>>16);
    CAN_FilterInitStructure.Filter_LowId          = (u16)(data>>0);
    data = (0xFFFF00<<3)|0x0007;
    CAN_FilterInitStructure.FilterMask_HighId     = (u16)(data>>16);
    CAN_FilterInitStructure.FilterMask_LowId      = (u16)(data>>0);
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);


	// 标准帧,松灵CAN接收：
	CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdMaskMode;
    CAN_FilterInitStructure.Filter_Scale          = CAN_Filter_32bitScale;
	CAN_FilterInitStructure.Filter_Num            = 3;
	// 计算标准帧寄存器值 (左移21位 + IDE=0, RTR=0)
	filter_std = (0x1A0 << 21) | 0x00;									// 标准帧：0x1A0 0x1A1 0x1A2
	CAN_FilterInitStructure.Filter_HighId         = (filter_std>>16);
    CAN_FilterInitStructure.Filter_LowId          = (filter_std>>0);	// IDE=0|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0xFF00;				// 掩码: 匹配高9位 + IDE/RTR
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0000;				// 忽略低16位
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
    
    CAN_FilterInitStructure.Filter_Num            = 4;
	// 计算标准帧寄存器值 (左移21位 + IDE=0, RTR=0)
	filter_std = (0x2A0 << 21) | 0x00;									// 标准帧：0x2A0 0x2A1 0x2A2
	CAN_FilterInitStructure.Filter_HighId         = (filter_std>>16);
    CAN_FilterInitStructure.Filter_LowId          = (filter_std>>0);	// IDE=0|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0xFF00;				// 掩码: 匹配高9位 + IDE/RTR
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0000;				// 忽略低16位
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
    
    CAN_FilterInitStructure.Filter_Num            = 5;
	// 计算标准帧寄存器值 (左移21位 + IDE=0, RTR=0)
	filter_std = (0x1C3 << 21) | 0x00;									// 标准帧：0x1C3  -> ECU群发广播指令
	CAN_FilterInitStructure.Filter_HighId         = (filter_std>>16);
    CAN_FilterInitStructure.Filter_LowId          = (filter_std>>0);	// IDE=0|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0xFFE0;				// 掩码: 匹配高11位 + IDE/RTR
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0000;				// 忽略低16位
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
    
    CAN_FilterInitStructure.Filter_Num            = 6;
	// 计算标准帧寄存器值 (左移21位 + IDE=0, RTR=0)
	filter_std = (0x1D0 << 21) | 0x00;									// 标准帧：0x1D0  0x1D1 0x1D2
	CAN_FilterInitStructure.Filter_HighId         = (filter_std>>16);
    CAN_FilterInitStructure.Filter_LowId          = (filter_std>>0);	// IDE=0|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0xFF80;				// 掩码: 匹配高9位 + IDE/RTR
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0000;				// 忽略低16位
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);
	
	CAN_FilterInitStructure.Filter_Num            = 7;
	// 计算标准帧寄存器值 (左移21位 + IDE=0, RTR=0)
	filter_std = (0x0A0 << 21) | 0x00;									// 标准帧：0x0A0 0x0A1 0x0A2
	CAN_FilterInitStructure.Filter_HighId         = (filter_std>>16);
    CAN_FilterInitStructure.Filter_LowId          = (filter_std>>0);	// IDE=0|RTR=0|0
    CAN_FilterInitStructure.FilterMask_HighId     = 0xFF80;				// 掩码: 匹配高9位 + IDE/RTR
    CAN_FilterInitStructure.FilterMask_LowId      = 0x0000;				// 忽略低16位
	CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);

	// BW protocol
	CAN_FilterInitStructure.Filter_Mode           = CAN_Filter_IdListMode;
    data = (BW_ID_EXT_PC<<3)|0x04;	
    CAN_FilterInitStructure.Filter_Num            = 10;
    CAN_FilterInitStructure.FilterMask_HighId     = data>>16;
    CAN_FilterInitStructure.FilterMask_LowId      = data;
		data = (BW_ID_SWITCH<<3)|0x0004;									// BW upgrade2
    CAN_FilterInitStructure.Filter_HighId         = data>>16;		// reserve
    CAN_FilterInitStructure.Filter_LowId          = data;
    CAN_FilterInitStructure.Filter_FIFOAssignment = CAN_FIFO0;
    CAN_InitFilter(&CAN_FilterInitStructure);


    /* IT Configuration for CAN */
    CAN_INTConfig(AND_CAN_DEV, CAN_INT_FMP0, ENABLE);
    CAN_INTConfig(AND_CAN_DEV, CAN_INT_FMP1, ENABLE);

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
	CAN_INTConfig(AND_CAN_DEV, CAN_INT_FMP0, ENABLE);
#endif
}

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

void CAN_RX0_IRQHandler_CallBack(void)
{
	CanRxMessage RxMsg;

	CAN_ReceiveMessage(AND_CAN_DEV, CAN_FIFO0, &RxMsg);
	queue_push_back(AndRxQueue, &RxMsg);
}
void CAN_RX1_IRQHandler_CallBack(void)
{
	CanRxMessage RxMsg;

	CAN_ReceiveMessage(AND_CAN_DEV, CAN_FIFO1, &RxMsg);
	queue_push_back(AndRxQueue, &RxMsg);
}

void AndHwCanInit(void)
{
    CAN_InitType CAN_InitStructure;
     /* Struct init*/
    CAN_InitStruct(&CAN_InitStructure);

    /* enable CAN clk */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_CAN, ENABLE);

    /* CAN register deinit */
    CAN_DeInit(AND_CAN_DEV);

    /* CAN cell init */
    CAN_InitStructure.TTCM              = DISABLE;			// 禁用时间触发,禁用时间戳同步
    CAN_InitStructure.ABOM              = ENABLE;			// 自动离线恢复,总线故障时自动恢复,避免总线错误导致永久离线 
    CAN_InitStructure.AWKUM             = DISABLE;			// 禁用自动唤醒,总线活动时自动退出睡眠模式
    CAN_InitStructure.NART              = DISABLE;			// 禁用自动重传,允许失败帧自动重传
    CAN_InitStructure.RFLM              = DISABLE;			// 禁用新报文覆盖旧报文,防止接收FIFO满时丢弃新报文
    CAN_InitStructure.TXFP              = ENABLE;			// 0:按ID优先级发送  1:FIFO顺序发送（时序可控）
    CAN_InitStructure.OperatingMode     = CAN_Normal_Mode;	// 标准通信模式 
#ifdef CAN_LOOP
    CAN_InitStructure.OperatingMode     = CAN_LoopBack_Mode;// [环回模式: 实现自测试]
#endif
    CAN_InitStructure.RSJW              = CAN_BIT_RSJW;
    CAN_InitStructure.TBS1              = CAN_BIT_BS1;
    CAN_InitStructure.TBS2              = CAN_BIT_BS2;
    CAN_InitStructure.BaudRatePrescaler = CAN_BAUDRATEPRESCALER;
    /*Initializes the CAN */
    CAN_Init(AND_CAN_DEV, &CAN_InitStructure);

    CAN_Filter_Init();

//    CAN_NVIC_Config();
}
static u8 TimerAndCanFlag;
void AndCanTimerCallBack(u8 ticks)
{
	static u8 Div10ms = 0;

	TimerAndCanFlag = 1;

	Div10ms += ticks;
	if (Div10ms < 10)
		return;
	Div10ms -= 10;

	AndCanInfo.TxPeriodTimer[0]++;
	AndCanInfo.TxPeriodTimer[1]++;
	AndCanInfo.TxPeriodTimer[2]++;
	AndCanInfo.TxPeriodTimer[3]++;
	AndCanInfo.TxPeriodTimer[4]++;
	AndCanInfo.TxPeriodTimer[5]++;
	if (AndCanInfo.BroadCaseTimer)
	{
		AndCanInfo.BroadCaseTimer--;
	}
	if (AndCanInfo.HeartBeatTimer)
	{
		AndCanInfo.HeartBeatTimer--;
	}
}


void SL_CanBmsMsgTransmit(BMS_STD_E index)
{
	u8 i = 0;
	u8  MyId = 0;
	u8  data8 = 0;
	u16 data16 = 0;
	u32 data32 = 0;
	static u32 frame_index = 0;
	CanTxMessage TxMsg = {0};
	
	MyId = MultiCanAddr();	
//	TxMsg.StdId  = SL_CAN_ECU_ID1 + MyId;			// 11位标准ID (根据并包分配ID获取)
	TxMsg.IDE   = CAN_ID_STD;           			// 标准帧标识符
	TxMsg.RTR   = CAN_RTRQ_DATA;					// 数据帧
	TxMsg.DLC   = 8;                    			// 发送8字节数据
	
	switch (index)
	{
		case STD_BMS_INFO1:
			TxMsg.StdId  = SL_CAN_ECU_ID1 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x01;					// 第一包数据
			TxMsg.Data[1] = BmsGetSystemState();	// 电池工作状态
			TxMsg.Data[2] = BmsGetWarnState();		// 电池保护信息
			
			data16 = BatData.VolTotal/10;			// 电池总电压,单位为10mV
			TxMsg.Data[3] = (u8)data16;
			TxMsg.Data[4] = (u8)(data16 >> 8);
			
			data16 = (u16)(AfeGetCurrentFilt()/10);	// 电池总电流,单位为10mA
			TxMsg.Data[5] = (u8)data16;
			TxMsg.Data[6] = (u8)(data16 >> 8);
			TxMsg.Data[7] = (u8)SocInfo.SocS;
			break;
			
		case STD_BMS_INFO2:
			TxMsg.StdId  = SL_CAN_ECU_ID1 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x02;					// 第二包数据
			data16 = BatData.VolMax;
			TxMsg.Data[1] = (u8)data16;
			TxMsg.Data[2] = (u8)(data16 >> 8);
			
			data16 = BatData.VolMin;
			TxMsg.Data[3] = (u8)data16;
			TxMsg.Data[4] = (u8)(data16 >> 8);
			
			data16 = BatNtcTempMax;
			TxMsg.Data[5] = (u8)data16;
			TxMsg.Data[6] = (u8)(data16 >> 8);
			
			data8 = 0;
			
			if(BatStatus.Bits.PDmos && AfeStatus.Bits.PDSG_EN)
			{
				data8 +=1;
			}
			if(BatStatus.Bits.Dmos && AfeStatus.Bits.DSG_EN)
			{
				data8 +=2;
			}
			if(BatStatus.Bits.Cmos && AfeStatus.Bits.CHG_EN)
			{
				data8 +=4;
			}
			TxMsg.Data[7] = data8;
			break;
	
		case STD_BMS_INFO3:
			TxMsg.StdId  = SL_CAN_ECU_ID1 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x03;					// 第三包数据
			data16 = BatNtcTempMin;
			TxMsg.Data[1] = (u8)data16;
			TxMsg.Data[2] = (u8)(data16 >> 8);
			
			data16 = MosNtcTemp[0];
			TxMsg.Data[3] = (u8)data16;
			TxMsg.Data[4] = (u8)(data16 >> 8);
			
			data16 = MosNtcTemp[1];
			TxMsg.Data[5] = (u8)data16;
			TxMsg.Data[6] = (u8)(data16 >> 8);
			
			TxMsg.Data[7] = GetDetId(); //电池包插槽ID
			break;
		case STD_BMS_INFO4:
            TxMsg.StdId  = SL_CAN_ECU_ID1 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x04;					// 第四包数据
        
            /* 电池保护状态1 */
            data8 = 0;
            if(BatStatus.Bits.ChgOV || BatStatus.Bits.ChgFC)
                data8 |= 0x01;
            if(BatStatus.Bits.DisUV)
                data8 |= 0x02;
            if(BatStatus.Bits.BatErr)
                data8 |= 0x04;
            if(BatStatus.Bits.DisOC)
                data8 |= 0x08;
            if(BatStatus.Bits.ChgOC)
                data8 |= 0x10;
            if(BatStatus.Bits.DisSC)
                data8 |= 0x20;
            if(BatStatus.Bits.DisOT)
                data8 |= 0x40;
            if(BatStatus.Bits.ChgOT && BatStatus.Bits.ChgPlugin)
                data8 |= 0x80;
            TxMsg.Data[1] = data8;
            
            /* 电池保护状态2 */
            data8 = 0;
            if(BatStatus.Bits.ChgMOT && BatStatus.Bits.ChgPlugin)
                data8 |= 0x01;
            if(BatStatus.Bits.DisMOT)
                data8 |= 0x02;
            if(BatStatus.Bits.AfeErr)
                data8 |= 0x04;
            if(BatStatus.Bits.CanErr)
                data8 |= 0x08;
           //	if(BatStatus.Bits.xxx)  //RTC通讯故障
           //       data8 |= 0x10;
           //	if(BatStatus.Bits.xxx)  //外部flash故障
           //       data8 |= 0x20;
            if(BatStatus.Bits.CellNtcErr)
                data8 |= 0x40;
            if(BatStatus.Bits.DNtcErr || BatStatus.Bits.CNtcErr)
                data8 |= 0x80;
            TxMsg.Data[2] = data8;
            
            /* 电池保护状态3 */
            data8 = 0;
//            if(BatStatus.Bits.FuseBlow)
//                data8 |= 0x01;
            if(BatStatus.Bits.ChgER)
                data8 |= 0x02;
           //	if(BatStatus.Bits.xxx)  //并包故障
           //       data8 |= 0x04;
            if(BatStatus.Bits.CMosErr)
                data8 |= 0x08;
            if(BatStatus.Bits.DMosErr)
                data8 |= 0x10;
            TxMsg.Data[3] = data8;
						
						TxMsg.Data[4] = frame_index&0xFF;
						TxMsg.Data[5] = (frame_index>>8)&0xFF;
						TxMsg.Data[6] = (frame_index>>16)&0xFF;
						TxMsg.Data[7] = (frame_index>>24)&0xFF;
            TxMsg.DLC   = 8;
						
						if(frame_index == 0xFFFFFFFF)
						{
							frame_index = 0;
						}else
						{
							frame_index++;
						}
            break;
		case STD_BMS_ACK01:							// 指定BMS发关机指令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x01;
			TxMsg.Data[1] = 0x5A;
			TxMsg.Data[2] = 0xA5;
			TxMsg.DLC   = 3;						// 发送3字节数据
			break;	
			
		case STD_BMS_ACK02:							// 指定BMS发复位指令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x02;
			TxMsg.Data[1] = 0x4B;
			TxMsg.Data[2] = 0xB4;
			TxMsg.DLC   = 3;						// 发送3字节数据
			break;
			
		case STD_BMS_ACK03:							// 指定BMS发查询版本命令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;	// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x03;
			
			TxMsg.Data[1] = *((u8*)SW_VERSION + 11);	// 软件版本
			TxMsg.Data[2] = *((u8*)SW_VERSION + 12);	// 软件版本
			
			TxMsg.Data[3] = '0';//DataFlashAtOnceSave.HwCode[0]; //*((u8*)HW_VERSION + 10);	// 硬件版本
			TxMsg.Data[4] = 'A';//DataFlashAtOnceSave.HwCode[1]; //*((u8*)HW_VERSION + 11);	// 硬件版本
			TxMsg.DLC   = 5;							// 发送5字节数据
			break;
			
		case STD_BMS_ACK04:								// 查询各节电芯电压
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x04;
			for (i=0; i<CELL_NUM*2/6; i++)
    		{        		
    			TxMsg.Data[1] = i+1;
				TxMsg.Data[2] = (u8)BatData.Bat[i*3+0].Vol; 
                TxMsg.Data[3] = (u8)(BatData.Bat[i*3+0].Vol >> 8);     
                TxMsg.Data[4] = (u8)BatData.Bat[i*3+1].Vol;
                TxMsg.Data[5] = (u8)(BatData.Bat[i*3+1].Vol >> 8);     
                TxMsg.Data[6] = (u8)BatData.Bat[i*3+2].Vol;
                TxMsg.Data[7] = (u8)(BatData.Bat[i*3+2].Vol >> 8);      
				TxMsg.DLC = 8;   
				queue_push_back(AndTxQueue, &TxMsg);
			}
            TxMsg.Data[1] = i+1;
			TxMsg.Data[2] = (u8)BatData.Bat[12].Vol;
            TxMsg.Data[3] = (u8)(BatData.Bat[12].Vol >> 8);
            TxMsg.Data[4] = 0;
            TxMsg.Data[5] = 0;
            TxMsg.Data[6] = 0;
            TxMsg.Data[7] = 0;
			TxMsg.DLC = 8;   
			break;	
			
		case STD_BMS_ACK05:								// 查询各电芯温度&Mos温度
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x05;
			TxMsg.Data[1] = 0x01;
			TxMsg.Data[2] = (u8)BatNtcTemp[0];
            TxMsg.Data[3] = (u8)(BatNtcTemp[0] >> 8);
            TxMsg.Data[4] = (u8)BatNtcTemp[1];
            TxMsg.Data[5] = (u8)(BatNtcTemp[1] >> 8);
			TxMsg.Data[6] = (u8)BatNtcTemp[2];
            TxMsg.Data[7] = (u8)(BatNtcTemp[2] >> 8);
            TxMsg.DLC = 8;   
			queue_push_back(AndTxQueue, &TxMsg);
			
            TxMsg.Data[1] = 0x02;
			TxMsg.Data[2] = (u8)MosNtcTemp[0];
            TxMsg.Data[3] = (u8)(MosNtcTemp[0] >> 8);
            TxMsg.Data[4] = (u8)MosNtcTemp[1];
            TxMsg.Data[5] = (u8)(MosNtcTemp[1] >> 8);
            TxMsg.DLC = 6;  
			break;
			
		case STD_BMS_ACK06:								// 查询总上电的次数&休眠次数
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x06;
			data32 = DataFlashPowerDownSave.Soc.Rmc;	//DataFlashPowerDownSave.PowerOnCnt;	// 上电次数
			TxMsg.Data[1] = (u8)data32;
			TxMsg.Data[2] = (u8)(data32 >> 8);
            TxMsg.Data[3] = (u8)(data32 >> 16);
            
			data32 = DataFlashPowerDownSave.Soc.Fcc;	//DataFlashPowerDownSave.WakeUpCnt;	// 休眠次数
            TxMsg.Data[4] = (u8)data32;
            TxMsg.Data[5] = (u8)(data32 >> 8);
            TxMsg.Data[6] = (u8)(data32 >> 16);
            TxMsg.Data[7] = 0;
            TxMsg.DLC = 8;
			break;
		case STD_BMS_ACK07:								// 查询SOH&循环次数
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x07;

			TxMsg.Data[1] = DataFlashPowerDownSave.Soc.Soh;	// SOH
			data16 = DataFlashPowerDownSave.Soc.Cycle;	// 循环次数
		  TxMsg.Data[2] = (u8)data16;
			TxMsg.Data[3] = (u8)(data16 >> 8);         
		
		  data16 = DataFlashPowerDownSave.Soc.RmcDisp;	// 剩余容量
		  TxMsg.Data[4] = (u8)data16;
			TxMsg.Data[5] = (u8)(data16 >> 8);      
		
            TxMsg.DLC = 6;
			break;		
		case STD_BMS_ACK08:								// 查询二维码 --> 多包发送
		{	
			u8 i;
        	u16 len;
        	
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			
			len = 6;
			TxMsg.DLC = 8;
        	for (i=0; i<len; i++)
        	{
        		TxMsg.Data[0] = 0x08;					// 
        		TxMsg.Data[1] = i+1;					// index
				if(i == 5)
				{
					memcpy(&TxMsg.Data[2], DataFlashAtOnceSave.QRCode+(i*6), 2);
					TxMsg.Data[4] = 0;
					TxMsg.Data[5] = 0;
					TxMsg.Data[6] = 0;
					TxMsg.Data[7] = 0;
				}
				else
				{
					memcpy(&TxMsg.Data[2], DataFlashAtOnceSave.QRCode+(i*6), 6);
				}
        		queue_push_back(AndTxQueue, &TxMsg);
        	}
        	
        }
		return;											// 设置硬件版本
    case STD_BMS_ACK09:		
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x09;

			AtOnceSaveDataRead();
			TxMsg.Data[1] = '0';						//DataFlashAtOnceSave.HwCode[0];	
			TxMsg.Data[2] = 'A';							//DataFlashAtOnceSave.HwCode[1];
      		TxMsg.DLC = 3;			
			break;
			
		case STD_BMS_ACK10:								// 查询Log记录 --> 多包发送
		{	
//			u8 i;
//        	u16 len;
//        	//u8 Log[sizeof(BMS_DATA_LOG)] = {0};
//        	
//        	//BmsLogRead(BmsLogIndex, Log);				// 读取当前日志序号对应的log信息
//			
//					u8 Log[sizeof(BMS_LOG)];
//					EfLogRead(BmsLogIndex, Log);		// 读取当前日志序号对应的log信息
//        	
//			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
//			TxMsg.Data[0] = 0x0A;						// 查询Log记录
//			TxMsg.Data[1] = 0x01;						// 第一帧数据
//			data16 = BmsLogIndex;						// 日志序号,低字节在前,从0开始,0表示最新的一条记录。
//			TxMsg.Data[2] = (u8)data16;	
//			TxMsg.Data[3] = (u8)(data16 >> 8);
//			data16 = sizeof(BMS_LOG); //sizeof(BMS_DATA_LOG);				// 标识后面有效数据个数
//			TxMsg.Data[4] = (u8)data16;
//			TxMsg.Data[5] = (u8)(data16 >> 8);
//			TxMsg.DLC = 6;
//            queue_push_back(AndTxQueue, &TxMsg);
//			
//			len = (sizeof(BMS_LOG)+5)/6; //(sizeof(BMS_DATA_LOG)+5)/6;
//			TxMsg.DLC = 8;
//        	for (i=0; i<len; i++)
//        	{
//        		TxMsg.Data[0] = 0x0A;					// 查询Log记录
//        		TxMsg.Data[1] = i+2;					// index
//        		memcpy(&TxMsg.Data[2], Log+(i*6), 6);
//        		queue_push_back(AndTxQueue, &TxMsg);
//        	}
        	
        }
		return;											// 多包发送,直接退出
		
		case STD_BMS_ACK11:								// 指定BMS发关机指令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x0B;
			TxMsg.Data[1] = 0xB1;
			TxMsg.Data[2] = 0xB2;
			TxMsg.DLC   = 3;							// 发送3字节数据
			break;	
			
		case STD_BMS_ACK12:								// 指定BMS发RTC校准命令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x0C;
		
		  TxMsg.Data[1] = AndCanRxMsgNow.Data[1];
			TxMsg.Data[2] = AndCanRxMsgNow.Data[2];
			TxMsg.Data[3] = AndCanRxMsgNow.Data[3];
			TxMsg.Data[4] = AndCanRxMsgNow.Data[4];
			TxMsg.Data[5] = AndCanRxMsgNow.Data[5];
			TxMsg.Data[6] = AndCanRxMsgNow.Data[6];
			TxMsg.Data[7] = AndCanRxMsgNow.Data[7];
		
			TxMsg.DLC   = 8;							// 发送8字节数据
			break;
		
		case STD_BMS_ACK13:								// 指定BMS发查询版本命令
			TxMsg.StdId  = SL_CAN_ECU_ID3 + MyId;		// 11位标准ID:0x1B0 (根据并包分配ID获取)
			TxMsg.Data[0] = 0x0D;
			
			TxMsg.Data[1] = 'B'; //电池包型号BW14S4P
		  	TxMsg.Data[2] = 'W';
		  	TxMsg.Data[3] = '1';
			TxMsg.Data[4] = '3';
		  	TxMsg.Data[5] = 'S';
		  	TxMsg.Data[6] = '3';
		  	TxMsg.Data[7] = 'P';
		
		  	TxMsg.DLC   = 8;							// 发送8字节数据
			break;
		default:
			return;
	}
	
	queue_push_back(AndTxQueue, &TxMsg);
}


void SlCanBmsReceiveProcess(void)
{
	u8 cmd = 0;
	u8 MyId1 = 0;
	u8 MyId2 = 0;
	
	if(SlCanRxMsgNow.StdId == 0x1c3)
	{
		
		BatStatus.Bits.CanTO = 0;
    	CanToTimer = 0;
		
		if(AndCanRxMsgNow.Data[0] == 1)
		{
			if((AndCanRxMsgNow.Data[1] == 0x5A) && (AndCanRxMsgNow.Data[2] == 0xA5))
			{
				BatStatus.Bits.PowerOff = 1;
				BatStatus.Bits.ShutDown = 1;
				ShutDownTimer = 100;			// 延迟100*10ms = 1s关机下电
				BatStatus.Bits.KeyOn = 0;
			}
		}
		
		if(AndCanRxMsgNow.Data[0] == 2)
		{
			if((AndCanRxMsgNow.Data[1] == 0x4B) && (AndCanRxMsgNow.Data[2] == 0xB4))
			{
				BatStatus.Bytes.DisProt = 0;
			    BatStatus.Bytes.ChgProt = 0;
			    BatStatus.Bytes.HardFault = 0;
			//	BatStatus.Bytes.InputState = 0;
			//	BatStatus.Bytes.BmsState = 0;
				  SetDisResetState();
			}
		}
		
		if(AndCanRxMsgNow.Data[0] == 3)
		{
			if(AndCanRxMsgNow.Data[1] == 0x1) //充电器接入
			{
				Ecu_ChgPlugin = 1; 
                BatStatus.Bits.KeyOn = 1;			// -->强制设置上电激活标识
			}else
			{
				Ecu_ChgPlugin = 0; 
			}
		}
		
		if(AndCanRxMsgNow.Data[0] == 4)
		{
			if(AndCanRxMsgNow.Data[1] == 0x1) //主板进入休眠
			{
				SetSingleMosOnState();
				Ecu_Sleep = 1;
			}else if(AndCanRxMsgNow.Data[1] == 0x0) //主板退出休眠
			{
				ClearSingleMosOnState();
				Ecu_Sleep = 0;
			}
		}
		
		if(AndCanRxMsgNow.Data[0] == 5)
		{
			if((AndCanRxMsgNow.Data[1] == 0x5A) && (AndCanRxMsgNow.Data[2] == 0xB4))
			{
				//锁定输出
				Ecu_LockPower = 1;
			}
		}
	}
	
	if(SlCanRxMsgNow.StdId == 0x1d1 || SlCanRxMsgNow.StdId == 0x1d2)
	{
//		BatStatus.Bits.KeyOn = 1;					// -->强制设置上电激活标识
		BatStatus.Bits.CanTO = 0;
    	CanToTimer = 0;
    	
    	MyId1 = MultiCanAddr() & 0x0F;
    	MyId2 = (u8)SlCanRxMsgNow.StdId & 0x0F;
    	if(MyId1 != MyId2)							// 并包，区分ID
    	{
			return;
    	}
		
		cmd = AndCanRxMsgNow.Data[0];
		
		switch(cmd)
		{
			case 1:
				if((AndCanRxMsgNow.Data[1] == 0x5A) && (AndCanRxMsgNow.Data[2] == 0xA5))
				{
					BatStatus.Bits.PowerOff = 1;
					BatStatus.Bits.ShutDown = 1;
					ShutDownTimer = 100;			// 延迟100*10ms = 1s关机下电
					BatStatus.Bits.KeyOn = 0;
					SL_CanBmsMsgTransmit(STD_BMS_ACK01);
				}
				break;
		
			case 2:
				if((AndCanRxMsgNow.Data[1] == 0x4B) && (AndCanRxMsgNow.Data[2] == 0xB4))
				{
					BatStatus.Bytes.DisProt = 0;
				    BatStatus.Bytes.ChgProt = 0;
				    BatStatus.Bytes.HardFault = 0;
				//	BatStatus.Bytes.InputState = 0;
				//	BatStatus.Bytes.BmsState = 0;
					SL_CanBmsMsgTransmit(STD_BMS_ACK02);
				}
				break;
				
			case 3:
				SL_CanBmsMsgTransmit(STD_BMS_ACK03);
				break;
				
			case 4:
				SL_CanBmsMsgTransmit(STD_BMS_ACK04);
				break;
				
			case 5:
				SL_CanBmsMsgTransmit(STD_BMS_ACK05);
				break;
				
			case 6:
				SL_CanBmsMsgTransmit(STD_BMS_ACK06);
				break;	
			case 7:
				SL_CanBmsMsgTransmit(STD_BMS_ACK07);
				break;	
			case 8:// 多包发送
				SL_CanBmsMsgTransmit(STD_BMS_ACK08);
				break;	
			case 9: //设置硬件版本号			
//				DataFlashAtOnceSave.HwCode[0] = AndCanRxMsgNow.Data[1];
//				DataFlashAtOnceSave.HwCode[1] = AndCanRxMsgNow.Data[2];
				AtOnceSaveDataWrite();
			
				SL_CanBmsMsgTransmit(STD_BMS_ACK09);
				break;
				
			case 10:		// 多包发送
//				BmsLogIndex = BYTES_TO_INT(AndCanRxMsgNow.Data[2],AndCanRxMsgNow.Data[1]);		// 日志序号,低字节在前,从0开始,0表示最新的一条记录。
				SL_CanBmsMsgTransmit(STD_BMS_ACK10);
				break;
				
			case 11: //清除历史保护信息
//				if((AndCanRxMsgNow.Data[1] == 0xB1) && (AndCanRxMsgNow.Data[2] == 0xB2))
//				{
//					//BmsLogReset();
//					EfLogReset();
//					SL_CanBmsMsgTransmit(STD_BMS_ACK11);
//				}
				break;
			case 12: //RTC校准
//				RTC_TIME_S CalTime = {0};
//				
//				CalTime.year = AndCanRxMsgNow.Data[1];
//				CalTime.month = AndCanRxMsgNow.Data[2];
//				CalTime.week = AndCanRxMsgNow.Data[3];
//				CalTime.day = AndCanRxMsgNow.Data[4];
//				CalTime.hour = AndCanRxMsgNow.Data[5];
//				CalTime.minute = AndCanRxMsgNow.Data[6];
//				CalTime.second = AndCanRxMsgNow.Data[7];

//				RtcTimeSet(&CalTime);
//			
//				SL_CanBmsMsgTransmit(STD_BMS_ACK12);
				break;
			case 13: //查询电池包型号
				SL_CanBmsMsgTransmit(STD_BMS_ACK13);
				break;				
			default:
				break;	
				
		}
	}
}


void AndCanReceiveProcess(void)
{
	// 下线时，清空SwRxQueue
	if (0 == queue_pop_front(AndRxQueue, &AndCanRxMsgNow))
	{
		return;			// no data in queue
	}
	
	ComOverSleepTimer = 0;
	BatStatus.Bits.CanAlone = 0;
	

	AndCanInfo.RxFlag = 1;

	SlCanRxMsgNow.StdId = AndCanRxMsgNow.StdId;


	SlCanBmsReceiveProcess();
	
#ifdef MULTIPLE_PARALLEL	

	CanTxMessage TxMsg;
	
	if(MTCanReceiveProcess(AndCanRxMsgNow,&TxMsg))
	{
		queue_push_back(AndTxQueue, &TxMsg);
	}
    
#endif

#ifdef CAN_OTA_SL
	OtaSlReceiveProcess(AndCanRxMsgNow);
#endif

	//厂测指令
#if 1
	if (BW_ID_EXT_PC == AndCanRxMsgNow.ExtId)
	{
		BwCanBmsReceiveProcess(AndCanRxMsgNow);
	}
#endif

}

void AndCanTransmitProcess(void)
{
	CanTxMessage QueueMsg;


	// 下线时，清空SwTxQueue
	if (BatStatus.Bits.Sleep || BatStatus.Bits.PowerOff || BatStatus.Bits.ShutDown)
	{
		AndCanInfo.TxPeriodTimer[0] = 0;
		AndCanInfo.TxPeriodTimer[1] = 0;
		AndCanInfo.TxPeriodTimer[2] = 0;
		AndCanInfo.TxPeriodTimer[3] = 0;
		AndCanInfo.TxPeriodTimer[4] = 0;
		AndCanInfo.TxPeriodTimer[5] = 0;
	}
	else if ( /*(AndCanInfo.BroadCaseTimer > 0)
			 &&*/ (0 == IS_FACTORY_TEST_MODE())
			 && (0 == BMS_IS_UPGRADING()) )
	{
		if (AndCanInfo.TxPeriodTimer[1] > 300/TIMEBASE_LOOP)		// 200mS定时发送帧
		{
			AndCanInfo.TxPeriodTimer[1] = 0;
			
			SL_CanBmsMsgTransmit(STD_BMS_INFO1);

			SL_CanBmsMsgTransmit(STD_BMS_INFO2);
			SL_CanBmsMsgTransmit(STD_BMS_INFO3);
            SL_CanBmsMsgTransmit(STD_BMS_INFO4);
		}
	}
	else
	{
			//
	}
		

	// need test
	if (CAN_TxSTS_NoMailBox == CAN_TransmitState(AND_CAN_DEV))
	{
		return;
	}

	// pop msg from queue
	if (0 == queue_pop_front(AndTxQueue, &QueueMsg))
	{
		return;			// no data in queue
	}

	CAN_TransmitMessage(AND_CAN_DEV, &QueueMsg);
}

void AndCanInit(void)
{
	AndHwCanInit();
//////	BwCanBmsRegister(AndTxQueue);
	TimerAndCanFlag = 0;

    AndCanInfo.BroadCaseTimer = BMS_CAN_BC_OUT_TIME;
    
    BatStatus.Bits.CanTO = 0;
    CanToTimer = 0; 
}


void AndCanProcess(void)
{
	static u8 LastSleep = 0;

	if (0==BatStatus.Bits.Sleep && 1==LastSleep)
	{
		CAN_5V_ON();
		CAN_VIO_ON();
		AndCanInit();
	}
	LastSleep = BatStatus.Bits.Sleep;
	
	if (AndCanInfo.RxFlag == 0)
	{
		AndCanInfo.RxOutTimer++;
		if (AndCanInfo.RxOutTimer > 5000)	// 5S
		{
			AndCanInfo.RxOutTimer = 0;
		}
	}
	else
	{
		AndCanInfo.RxFlag = 0;
		AndCanInfo.RxOutTimer = 0;
	}
}

void SlCanTimeOut(void)
{
	if (BatStatus.Bits.CanTO == 0)
	{
		CanToTimer++;
		if (CanToTimer > CAN_TIME_OUT_DELAY)
		{
			BatStatus.Bits.CanTO = 1;
			
			ClearEcuChgPluginState();
			ClearEcuSleepState();
			ClearEcuLockPowerState();
		}
	}
}


void AndCanCtrl(void)
{
	AndCanReceiveProcess();

	if (TimerAndCanFlag == 0)		// 1ms tick
		return;
	TimerAndCanFlag = 0;
	
	if(BatStatus.Bits.PowerOff == 1) //关机不处理发送CAN数据
	{
		CanToTimer = 0;
		return;
	}

	AndCanProcess();

	AndCanTransmitProcess();

  	SlCanTimeOut();

}
#endif

#if 1
uint16_t CheckSum_CheckMemory_X(uint32_t checkAddr, uint32_t length)
{
    uint16_t crc_value = 0x0000;   
    uint16_t crc_byte;
    uint8_t *pos;

    pos = (uint8_t *)checkAddr;
    while (length--)     
    {  
        crc_byte = *pos++;  
        crc_value += crc_byte;
    }  
    return (crc_value) ;  
}

#endif

u8 ReadEcuChgPluginState(void)
{
	return Ecu_ChgPlugin;
}

u8 ReadEcuSleepState(void)
{
	return Ecu_Sleep;
}

u8 ReadEcuLockPowerState(void)
{
	return Ecu_LockPower;
}


void ClearEcuChgPluginState(void)
{
	 Ecu_ChgPlugin = 0x5A;
}

void ClearEcuSleepState(void)
{
	Ecu_Sleep = 0x5A;
}

void ClearEcuLockPowerState(void)
{
	Ecu_LockPower = 0x5A;
}

