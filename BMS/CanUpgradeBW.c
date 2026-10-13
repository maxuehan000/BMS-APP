/******************************************************************************

                  版权所有 (C), 2020-2030, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : CanUpgradeBW.c
  版 本 号   : 初稿
  作    者   : WuHongPing
  生成日期   : 2025年11月28日 
  最近修改   :
  功能描述   : 蓝微CAN升级模块
  函数列表   :
  修改历史   :
  1.日    期   : 2025年11月28日  
    作    者   : WuHongPing
    修改内容   : 创建文件

******************************************************************************/

#include "board.h"

/*数据包PACK定义: 包头(1帧Msg) + 负载数据128Bytes(16帧Msg) + 校验(1帧Msg) */
#define PACK_DATA_LEN               128           // 负载数据长度       
#define PACK_HEAD_LEN               8             // 包头长度
#define PACK_TAIL_LEN               8             // 包尾长度
#define PACK_MSG_NUM               (PACK_DATA_LEN/PACK_HEAD_LEN + 2)   // 由xx帧数据组成完整数据包
#define PACK_CRC16_NUM             (PACK_HEAD_LEN+PACK_DATA_LEN)  
#define PACK_TOTAL_LEN             (PACK_HEAD_LEN+PACK_DATA_LEN+PACK_TAIL_LEN)

#define FUN_CODE_UPGRADE            0xFA     // 升级功能码
typedef enum
{
	CC_REQUEST = 0,
	CC_LOADING,
	CC_UPDATE,
	CC_EXIT,
	CC_MAX
}CMD_CODE_UPGRADE;                            // 升级命令码

typedef enum
{
	AC_OK = 0,
	AC_BMS_CODE_ERR,
	AC_FW_LEN_OUT,
	AC_PACK_CRC_ERR,
	AC_TIME_OUT,
	AC_PACK_LEN_ERR,
	AC_PACK_SEQ_ERR,
	AC_FW_CRC_ERR,
	AC_FW_LEN_ERR,
	AC_FW_TAIL_ERR,
	AC_BMS_FAIL,
	AC_OTHER
}ACK_CODE;                                  // 回应码

#pragma pack(1)							// 单字节对齐
typedef struct{
	u8 SourAddr;		// 发送方地址
	u8 DestAddr;		// 接收方地址
	u8 FunCode;			// 消息类型
	u8 CmdCode;			// 命令类型
	u16 DataLen;		// 数据长度
	u16 MsgSeq;			// 序列号
}UpgradeRxMsgHead;		/* 协议包头结构 (8字节) */
static UpgradeRxMsgHead RxHdr;

typedef struct{
	u8 SourAddr;		// 发送方地址
	u8 DestAddr;		// 接收方地址
	u8 FunCode;			// 消息类型
	u8 CmdCode;			// 命令类型
	u8 AckData;		    // 应答码
	u8 Data[3];			// 预留数据
}UpgradeTxMsgHead;		/* 协议回应帧结构 (8字节) */
#pragma pack()	

#define FLASH_BUF_LEN 				(2048)
#pragma pack(4)							// 按4字节对齐
u8 UpgradeFlashBuf[FLASH_BUF_LEN];
#pragma pack()							// 取消4字节对齐

static u8 TimerUpgradeBWFlag;

/* 定义发送消息队列的指针，通过UpgradeBWRegister()指向其他协议已定义的消息队列。*/
static queue UpgradeTxQueue;

typedef struct
{
	u32 FwCrc32;                 // 固件信息中的校验值
	u32 FwSize;                  // 固件信息中的长度值
	u32 FwRxLen;                 // 接收固件的长度
	u8  PackRx[PACK_TOTAL_LEN];  // 单个数据包缓存
	u8  PackOutTimer;            // 数据接收超时计时器
}UpgradeBW;
static UpgradeBW UpgradeCtr;

static u8 DebugU8[3];                   // 调试打印buf


/*****************************************************************************
 函 数 名  : UpgradeBwTimerCallBack
 功能描述  : 升级处理时间片分配，间隔10ms置1
 输入参数  : ticks
 返 回 值  : void
*****************************************************************************/
void UpgradeBwTimerCallBack(u8 ticks)
{
	TimerUpgradeBWFlag = 1;
}

/*****************************************************************************
 函 数 名  : UpgradeBWReply
 功能描述  : BW CAN升级应答数据执行
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWReply(u8 Ack)
{
	CanTxMessage TxMsg = {0};

#ifdef CAN_2_STAND
	TxMsg.StdId = CAN_BWU_TX_ID;						// 11位标准ID
	TxMsg.IDE   = CAN_ID_STD;           				// 标准帧标识符
#else
	TxMsg.ExtId = CAN_BWU_TX_ID;						// 29位扩展ID
	TxMsg.IDE   = CAN_ID_EXT;           				// 扩展帧标识符
#endif
	
	TxMsg.RTR   = CAN_RTRQ_DATA;						// 数据帧
	TxMsg.DLC   = 8;                    				// 发送8字节数据
	
	TxMsg.Data[0] = BMS_UPGRADE_ADDR;
	TxMsg.Data[1] = RxHdr.SourAddr;
	TxMsg.Data[2] = RxHdr.FunCode;
	TxMsg.Data[3] = RxHdr.CmdCode;
	TxMsg.Data[4] = Ack;
	TxMsg.Data[5] = DebugU8[0];
	TxMsg.Data[6] = DebugU8[1];
	TxMsg.Data[7] = DebugU8[2];
	
	queue_push_back(UpgradeTxQueue, &TxMsg);
}

/*****************************************************************************
 函 数 名  : UpgradeBWPackDecode
 功能描述  : BW CAN升级接收数据包解析
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWPackDecode(void)
{
	static u16 NumSeq = 0;				// 固件包总数量
	static u16 PreSeq = 0;				// 已接收到当前固件包序列号				
	static u16 wDataIndex = 0;			// 已接收到当前固件包长度			
	u32_u8_union Converter;
	u32 Crc32_Calc, Crc32_Tail;
	u8 i;

	/* 考虑超时退回升级请求以便重新开始 */
	if (SlUpgradeTimer == 0)
	{
		NumSeq = 0;
		PreSeq = 0;
	}
	SlUpgradeTimer = FW_UPGRADE_OUT_TIME;

	memcpy(&RxHdr, UpgradeCtr.PackRx, PACK_HEAD_LEN);

	/* 1.固件升级请求处理 */
	if (RxHdr.CmdCode == CC_REQUEST)
	{
		PreSeq = 0;
		wDataIndex = 0;
		/* 固件识别码 */
		#define FW_ID_CODE      "BlueWay"
		for (i=0; i<7; i++)
		{
		//	if (UpgradeCtr.PackRx[8+i] != *(SW_VERSION+i))
			if (UpgradeCtr.PackRx[8+i] != *(FW_ID_CODE+i))
				break;
		}
		if (i < 7)
		{
			UpgradeBWReply(AC_BMS_CODE_ERR);
			return;
		}

		/* 固件长度提取 */
		Converter.uc8[0] = UpgradeCtr.PackRx[16];
	    Converter.uc8[1] = UpgradeCtr.PackRx[17];
	    Converter.uc8[2] = UpgradeCtr.PackRx[18];
	    Converter.uc8[3] = UpgradeCtr.PackRx[19];
		if (Converter.ul32 > UPDATE_FILE_MAX_LEN)
		{
			UpgradeBWReply(AC_FW_LEN_OUT);
			return;
		}
		UpgradeCtr.FwSize = Converter.ul32;

		/* MaxSeq提取 */
	#if 0
		NumSeq = ((u16)UpgradeCtr.PackRx[7]<<8) + UpgradeCtr.PackRx[6];
		if (NumSeq != (UpgradeCtr.FwSize+PACK_DATA_LEN-1)/PACK_DATA_LEN)
		{
			UpgradeBWReply(AC_FW_LEN_OUT);
			return;
		}
	#else
		NumSeq = (UpgradeCtr.FwSize+PACK_DATA_LEN-1)/PACK_DATA_LEN;
	#endif

		/* 固件CRC32提取 */
		Converter.uc8[0] = UpgradeCtr.PackRx[20];
	    Converter.uc8[1] = UpgradeCtr.PackRx[21];
	    Converter.uc8[2] = UpgradeCtr.PackRx[22];
	    Converter.uc8[3] = UpgradeCtr.PackRx[23];
		UpgradeCtr.FwCrc32 = Converter.ul32;

		UpgradeCtr.FwRxLen = 0;

		UpgradeBWReply(AC_OK);
	}
	/* 2.固件接收处理 */
	else if (RxHdr.CmdCode == CC_LOADING)
	{		
		if ((RxHdr.MsgSeq==PreSeq) && (RxHdr.MsgSeq<NumSeq))
		{
			PreSeq++;
			
			// 要求FLASH_BUF_LEN能被PACK_DATA_LEN整除，否则数据会有溢出或丢失风险
			if (wDataIndex <= FLASH_BUF_LEN-PACK_DATA_LEN)
				memcpy(&UpgradeFlashBuf[wDataIndex], &UpgradeCtr.PackRx[PACK_HEAD_LEN], PACK_DATA_LEN);
			wDataIndex += PACK_DATA_LEN;
			
			if ((wDataIndex >= FLASH_BUF_LEN) || (PreSeq == NumSeq))	// 满2K数据或者最后一包数据 
			{				
				wDataIndex = 0;
				
				HwClrWdt();
				HwDataUnlock();
				HwDataPageWriteRandom((FLASH_FW_BACKUP_ADDR+UpgradeCtr.FwRxLen), (u32*)&UpgradeFlashBuf, FLASH_BUF_LEN/4);
				UpgradeCtr.FwRxLen += FLASH_BUF_LEN;
				HwDataLock();
				HwClrWdt();				
			}
			
			UpgradeBWReply(AC_OK);
		}
		else if (RxHdr.MsgSeq < PreSeq)    // 重发帧响应ok，以便继续接收下一包数据
		{
			UpgradeBWReply(AC_OK);
		}
		else
		{
			UpgradeBWReply(AC_PACK_SEQ_ERR);
		}
	}
	/* 3.固件校验及重启处理 */
	else if (RxHdr.CmdCode == CC_UPDATE)
	{
		if ((PreSeq != NumSeq) || (NumSeq == 0))  // 避免未经过升级请求直接进入固件校验（耗时或者长度不确定出错）
		{
			UpgradeBWReply(AC_FW_LEN_ERR);
			return;
		}

		Crc32_Tail = *((volatile u32*)(FLASH_FW_BACKUP_ADDR+UpgradeCtr.FwSize-4));
		Crc32_Calc = SwCrc32(0xFFFFFFFF, (u8*)FLASH_FW_BACKUP_ADDR, UpgradeCtr.FwSize-4);
		if(Crc32_Calc != Crc32_Tail)
		{
			UpgradeBWReply(AC_FW_TAIL_ERR);
			return;
		}
		
		Crc32_Calc = SwCrc32(Crc32_Calc, (u8*)(FLASH_FW_BACKUP_ADDR+UpgradeCtr.FwSize-4), 4);
		if(Crc32_Calc != UpgradeCtr.FwCrc32)
		{
			UpgradeBWReply(AC_FW_CRC_ERR);
			return;
		}	

		// 固件校验成功，可以重启切换
		DataFlashPowerDownSave.BootMark = FLASH_BOOT_KEY;
		DataFlashPowerDownSave.UpgradeLen = UpgradeCtr.FwSize;
		DataFlashPowerDownSave.UpgradeCrc = UpgradeCtr.FwCrc32;			
			
		BootLoaderActive();

		UpgradeBWReply(AC_OK);

		UpgradeBWReply(AC_OK);      // 多发1帧以防上位机未接收到
	}
	/* 4.升级终止 */
	else if(RxHdr.CmdCode == CC_EXIT)
	{
		PreSeq = 0;
		NumSeq = 0;
		UpgradeBWReply(AC_OK);

		SlUpgradeTimer = 0;
	}
}

/*****************************************************************************
 函 数 名  : UpgradeBWMsgProcess
 功能描述  : 数据包接收，识别完整数据包后转decode解析处理
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWMsgProcess(CanRxMessage RxMsg)
{
	u16 CheckSum1,CheckSum2;	
	static u8 PackIndex = 0;

#ifdef CAN_2_STAND
	if(RxMsg.StdId != CAN_BWU_RX_ID)
		return;
#else
	if(RxMsg.ExtId != CAN_BWU_RX_ID)
		return;
#endif

	/* PackIndex超时重置 */
	if (UpgradeCtr.PackOutTimer == 0)
		PackIndex = 0;
	UpgradeCtr.PackOutTimer = PACK_OUT_TIME;

	/* 第一帧数据(获取包头&数据长度) */
	if(PackIndex == 0)
	{
		if(RxMsg.Data[1]==BMS_UPGRADE_ADDR && RxMsg.Data[2]==FUN_CODE_UPGRADE)	
		{
			if(RxMsg.Data[3] < CC_MAX)
			{
				PackIndex = 1;
				memcpy(&UpgradeCtr.PackRx[0], &RxMsg.Data[0], 8);                  // 拷贝包头数据:8个
			}
		}
	}
	/* 第x帧数据(接收负载数据) */
	else if(PackIndex < PACK_MSG_NUM-1)							
	{
		memcpy(&UpgradeCtr.PackRx[0+PackIndex*8], &RxMsg.Data[0], 8);					// 拷贝固件数据:8个
		PackIndex++;
	}
	/* 最后1帧数据(接收CRC16) */
	else if(PackIndex == PACK_MSG_NUM-1)											
	{
		memcpy(&UpgradeCtr.PackRx[0+PackIndex*8], &RxMsg.Data[0], 8);					// 拷贝校验数据:8个
		
		CheckSum1 = ((u16)RxMsg.Data[1]<<8) + RxMsg.Data[0];				    // 数据包的Crc16
		CheckSum2 = ModbusCrc16(&UpgradeCtr.PackRx[0], PACK_CRC16_NUM);				// 整包数据计算
		
		if(CheckSum1 == CheckSum2)
		{	
			UpgradeBWPackDecode();
		}
		else
		{
			UpgradeBWReply(AC_PACK_CRC_ERR);
		}
		PackIndex = 0;
	}
}

/*****************************************************************************
 函 数 名  : UpgradeBWTimeout
 功能描述  : 升级协议超时处理，含数据包内的帧超时和升级超时退出
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWTimeout(void)
{
	if (BMS_IS_UPGRADING())
	{
		if (UpgradeCtr.PackOutTimer > 0)
		{
			UpgradeCtr.PackOutTimer--;
		}
		
		if (SlUpgradeTimer > 0)
		{
			SlUpgradeTimer--;
		}
	}
}

/*****************************************************************************
 函 数 名  : UpgradeBWSchedule
 功能描述  : 蓝微升级协议模块，Main函数主循环调用
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWSchedule(void)
{
	if(!TimerUpgradeBWFlag)
	{
		return;
	}
	TimerUpgradeBWFlag = 0;
	
	
	UpgradeBWTimeout();
}

/*****************************************************************************
 函 数 名  : UpgradeBWRegister
 功能描述  : 蓝微升级协议模块，Main函数主循环调用
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void UpgradeBWRegister(queue TxQueue)
{
	UpgradeTxQueue = TxQueue;
}
