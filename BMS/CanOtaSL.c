/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : CanOtaSl.c
  版 本 号   : 初稿
  作    者   : Neiyang
  生成日期   : 2025年9月28日 
  最近修改   :
  功能描述   : 松灵CAN-OTA升级通讯文件
  函数列表   :
  修改历史   :
  1.日    期   : 2025年9月28日  
    作    者   : Neiyang
    修改内容   : 创建文件

******************************************************************************/

#include "board.h" 


#define SL_OTA_BUF_LEN (2048)
#define SL_OTA_FLASH_BASE_ADDR			UPDATE_BASE_ADDR 	//0x08010000

//static u8 OtaSlProcess = 0;				// 升级经常(步骤)
static u8 wPageIndex = 0;
//static u32 Crc32Value = 0;
//static u32 OtaSlFileRxSize = 0;			// 文件大小(固件长度)
static u32 OtaSlFileWrSize = 0;
static u8 OtaRxPack[152]= {0};  			// 接收数据包
static u8 OtaTxPack[48] = {0};			// 发送数据包
extern queue AndTxQueue;

#pragma pack(1)							// 单字节对齐
typedef struct{
	u8 fhdr1;		// 0xE1
	u8 fhdr2;		// 0x1E
	u8 sender;		// 发送方ID
	u8 rcver;		// 接收方ID
	u8 type;		// 消息类型
	u8 typecmd;		// 命令类型
	u16 len;		// 数据长度
	u16 seq;		// 序列号
} SlProtocolExtHdr;	// 协议头结构 (10字节)

SlProtocolExtHdr TxHdr;
SlProtocolExtHdr RxHdr;

// 固件头部(32字节)
typedef struct {
	u32 magic; //0x144F5441
	u32 img_size; //镜像大小
	char fw_ident[19]; //固件标识
	u32 file_crc32;// CRC32校验值
}SlFotaHeader;

SlFotaHeader RxFotaHeader;
#pragma pack()	


#pragma pack(4)							// 按4字节对齐
u8 OtaSlFlashBuf[2][SL_OTA_BUF_LEN];
#pragma pack()							// 取消4字节对齐

static u8 TimerOtaSlFlag;

/*****************************************************************************
 函 数 名  : OtaSlTimerCallBack
 功能描述  : OtaSl处理时间片分配，间隔10ms置1
 输入参数  : ticks
 返 回 值  : void
*****************************************************************************/
void OtaSlTimerCallBack(u8 ticks)
{
	TimerOtaSlFlag = 1;
} 


//计算MODBUS CRC16校验值
u16 crc16_modbus(const u8 *data, u16 length)
{
	u16 crc = 0xFFFF;
	u16 i, j;
	
	if(data == NULL || length == 0)
	{
		return crc;
	}
	
	for(i=0;i< length; i++)
	{
		crc ^= (u16)data[i];
		for(j=0;j<8;j++)
		{
			if(crc & 0x0001)
			{
				crc =(crc >>1)^ 0xA001;
			}
			else 
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}

/*****************************************************************************
 函 数 名  : crc32
 功能描述  : SL_OTA对整个.bin文件校验和的计算
 输入参数  : ticks
 返 回 值  : void
*****************************************************************************/
u32 crc32(u32 checkAddr, u32 len)
{
	u32 i;
	u32 crc = 0xffffffffL;
	
	u8 *pdata;
	u8 temp;
	pdata = (u8 *)checkAddr;

	while (len--)
	{
		temp = *pdata++;
		crc  ^= temp;
		for (i = 0 ; i <  8;  i++)
		{
			crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
		}
	}
	return crc ^0xffffffffL;
}

/*****************************************************************************
 函 数 名  : OtaSlVarInit
 功能描述  : 松灵CAN-OTA升级变量初始化
 输入参数  : void
 返 回 值  : void
*****************************************************************************/
void OtaSlVarInit(void)
{
	SlUpgradeTimer = 0;
//	OtaSlProcess = 0;
	//Crc32Value = 0;
	//OtaSlFileRxSize = 0;
	memset(&RxFotaHeader,0,sizeof(SlFotaHeader));
	OtaSlFileWrSize = 0;
	wPageIndex = 0;
	memset(&TxHdr,0,sizeof(SlProtocolExtHdr));
	memset(&RxHdr,0,sizeof(SlProtocolExtHdr));
} 

/*****************************************************************************
 函 数 名  : OtaSlWriteFlash
 功能描述  : 松灵CAN-OTA接收到2K数据后,写入到Flash备份缓存
 输入参数  : void page
 返 回 值  : void
*****************************************************************************/
void OtaSlWriteFlash(u8 page)
{
	HwClrWdt();
	HwDataUnlock();
	
	if(page == 0)  					// A Buf写入完成
	{
		HwDataPageWriteRandom((SL_OTA_FLASH_BASE_ADDR+ OtaSlFileWrSize),(u32*)&OtaSlFlashBuf[0][0], SL_OTA_BUF_LEN/4);
		OtaSlFileWrSize += SL_OTA_BUF_LEN;
		
		memset(&OtaSlFlashBuf[0][0],0,SL_OTA_BUF_LEN);
	}
	else if(page == 1) 				// B Buf写入完成
	{
		HwDataPageWriteRandom((SL_OTA_FLASH_BASE_ADDR + OtaSlFileWrSize),(u32*)&OtaSlFlashBuf[1][0], SL_OTA_BUF_LEN/4);
		OtaSlFileWrSize += SL_OTA_BUF_LEN;
		
		memset(&OtaSlFlashBuf[1][0],0,SL_OTA_BUF_LEN);
	}
	
	HwDataLock();
	HwClrWdt();


#if 0
	if(AndOtaInfo.DataCnt >= AndOtaInfo.FileSize && AndOtaInfo.FileSize != 0)  //烧录完毕
	{
		uint16_t crc_calc = CheckSum_CheckMemory_X(AND_OTA_FLASH_BASE_ADDR, AndOtaInfo.FileSize);

		//升级文件烧录成功，则可以重启切换
		BootLoaderActive();
		if(crc_calc == AndOtaInfo.FileCrc16)
		{
			DataFlashPowerDownSave.BootMark = FLASH_BOOT_BWK;
			DataFlashPowerDownSave.UpgradeLen = AndOtaInfo.FileSize;
			DataFlashPowerDownSave.UpgradeCrc = crc_calc;	
		}
	}
	HwClrWdt();
#endif

}

/*****************************************************************************
 函 数 名  : OtaSlSendManage
 功能描述  : 松灵CAN-OTA升级发送数据处理
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void OtaSlSendManage(void)
{
	u8 i;
	u16 Checksum;
	u16 DataLen;
	
	CanTxMessage TxMsg = {0};
	
	TxMsg.StdId  = 0x0b0+MultiCanAddr();//0x0b1;								// 11位标准ID
	TxMsg.IDE   = CAN_ID_STD;           				// 标准帧标识符
	TxMsg.RTR   = CAN_RTRQ_DATA;						// 数据帧
	TxMsg.DLC   = 8;                    				// 发送8字节数据
	
	DataLen = (OtaTxPack[6] | (OtaTxPack[7] << 8)) & 0xFFFF;
	
	Checksum = crc16_modbus(&OtaTxPack[0], DataLen+10);			// 整包数据：10+32=42个
	OtaTxPack[DataLen+10] = LOBYTE(Checksum);
	OtaTxPack[DataLen+11] = HIBYTE(Checksum);
	
	DataLen += 12;
	
	//发送前面完整的8字节
	for(i=0; ((i+1)*8) <= DataLen; i++)
	{
		memcpy(&TxMsg.Data[0], &OtaTxPack[0+i*8], 8);
		queue_push_back(AndTxQueue, &TxMsg);
		
	}
	
	//发送最后一帧
	if((i*8) != DataLen)
	{
		TxMsg.DLC   = DataLen - i*8; 
		memcpy(&TxMsg.Data[0], &OtaTxPack[0+i*8], TxMsg.DLC);
		queue_push_back(AndTxQueue, &TxMsg);	
	}
	
}


/*****************************************************************************
 函 数 名  : OtaSlSendManage
 功能描述  : 松灵CAN-OTA升级发送数据处理
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void OtaSlAbortCmd(s8 err)
{
	TxHdr.fhdr1 = SL_OTA_HEAD1;
	TxHdr.fhdr2 = SL_OTA_HEAD2;
	TxHdr.sender = RxHdr.rcver;
	TxHdr.rcver = RxHdr.sender;
	TxHdr.type = 0xFA;
	TxHdr.typecmd = 0x03;
	TxHdr.len = 0x0001;
	TxHdr.seq = 0x0001;
	
	memset(&OtaTxPack,0,sizeof(OtaTxPack));
	memcpy(&OtaTxPack[0], &TxHdr.fhdr1, sizeof(SlProtocolExtHdr));
	OtaTxPack[10] = err;								// ACK:默认发送成功
	
	OtaSlSendManage();
}

/*****************************************************************************
 函 数 名  : OtaSlReceiveDecode
 功能描述  : 松灵CAN-OTA升级接收数据处理
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void OtaSlReceiveDecode(void)
{
	//static u16 MaxSeq = 0;
	static u16 PreSeq = 0;								// 上1个包序列号
	static u16 wDataIndex = 0;							// 已接收到固件数据
	static u32 dwTotalRevLen = 0;          //接收到的总长度
//	u32_u8_union Converter;
//	u16    rev_crc16 = 0,verify_crc16;
	u32    rev_datalen = 0,next_datalen=0;
		
	RxHdr.fhdr1 = OtaRxPack[0];
	RxHdr.fhdr2 = OtaRxPack[1];
	RxHdr.sender = OtaRxPack[2];
	RxHdr.rcver = OtaRxPack[3];
	RxHdr.type = OtaRxPack[4];
	RxHdr.typecmd = OtaRxPack[5];
	RxHdr.len = BYTES_TO_INT(OtaRxPack[7],OtaRxPack[6]);// 此包数据data个数
	RxHdr.seq = BYTES_TO_INT(OtaRxPack[9],OtaRxPack[8]);// 包序列号
//	rev_crc16 = BYTES_TO_INT(OtaRxPack[43],OtaRxPack[42]);// 帧crc
//	verify_crc16 = crc16_modbus(&OtaRxPack[0],42);
//	
//	//校验CRC
//	if(rev_crc16 != verify_crc16)
//	{
//			OtaSlAbortCmd(FAIL_TRANSPORT_ERROR);
//			OtaSlProcess = 0;
//			return;
//	}
	
	if((RxHdr.type == 0xAA) && (RxHdr.typecmd == 0x08))
	{
//		OtaSlProcess = 1;								// 升级流程：第1步-读取设备版本
		
		if(BatData.VolMin <= DIS_UNDER_VOL_RE) 			// 低电量禁止升级
		{
			OtaSlAbortCmd(FAIL_BATTERY_LOW);
//			OtaSlProcess = 0;
			return;
			
		}
		
		TxHdr.fhdr1 = SL_OTA_HEAD1;
		TxHdr.fhdr2 = SL_OTA_HEAD2;
		TxHdr.sender = RxHdr.rcver;
		TxHdr.rcver = RxHdr.sender;
		TxHdr.type = RxHdr.type;
		TxHdr.typecmd = RxHdr.typecmd;
		TxHdr.len = 0x5;								// 此包数据data个数:5
		TxHdr.seq = 0x0001;								// 系列号
		
		memset(&OtaTxPack,0,sizeof(OtaTxPack));
		memcpy(&OtaTxPack[0], &TxHdr.fhdr1, sizeof(SlProtocolExtHdr));
		
		OtaTxPack[10] = 'B';							// 
		OtaTxPack[11] = 'W';							// 
		OtaTxPack[12] = '_';							// 
		OtaTxPack[13] = *((u8*)SW_VERSION + 11);		// 软件版本
		OtaTxPack[14] = *((u8*)SW_VERSION + 12);		// 软件版本
		
		OtaSlSendManage();
	}
	else if(RxHdr.type == 0xFA)
	{
		if(RxHdr.typecmd == 0x04)
		{
//			OtaSlProcess = 2;							// 升级流程：第2步-准备升级
			
		}
		else if(RxHdr.typecmd == 0x00)
		{
//			OtaSlProcess = 3;							// 升级流程：第3步-启动升级
			PreSeq = 0;
		}
		else if(RxHdr.typecmd == 0x01)
		{
//			OtaSlProcess = 4;							// 升级流程：第3步-数据传输
			
			if(RxHdr.seq == 1)							// --> 固件头部(32Byte)
			{

				memcpy(&RxFotaHeader,&OtaRxPack[10],sizeof(RxFotaHeader));
				if(RxFotaHeader.magic != 0x144F5441)		// 魔数校验:0x144F5441
				{
					OtaSlAbortCmd(FAIL_IMAGE_ERROR);
					return;
				}
		    	
				
				if(false)//RxFotaHeader.fw_ident[11] != DataFlashAtOnceSave.HwCode[0] ||  RxFotaHeader.fw_ident[12] != DataFlashAtOnceSave.HwCode[1])
				{
					OtaSlAbortCmd(FAIL_IMAGE_ERROR);
					return;
				}
				
				PreSeq = RxHdr.seq;
				wPageIndex = 0;
				wDataIndex = 0;
				OtaSlFileWrSize = 0;
				dwTotalRevLen = 0;
			}
			else
			{
				if(RxHdr.seq == PreSeq+1)
				{
					PreSeq = RxHdr.seq;
					
					rev_datalen = RxHdr.len;
					next_datalen = 0;
					
					if(wDataIndex + rev_datalen <= SL_OTA_BUF_LEN)
					{
						memcpy(&OtaSlFlashBuf[wPageIndex][wDataIndex],&OtaRxPack[10],rev_datalen);
						wDataIndex += rev_datalen;
						dwTotalRevLen += rev_datalen;
					}else{
						next_datalen = (wDataIndex + rev_datalen) - SL_OTA_BUF_LEN;
						memcpy(&OtaSlFlashBuf[wPageIndex][wDataIndex],&OtaRxPack[10],rev_datalen-next_datalen);
						
						wDataIndex += (rev_datalen-next_datalen);
						dwTotalRevLen += (rev_datalen-next_datalen);
					}

					if(wDataIndex >= SL_OTA_BUF_LEN)	// 2K数据 
					{
						OtaSlWriteFlash(wPageIndex);
						if(wPageIndex == 0)
						{
							wPageIndex = 1;
						}
						else
						{
							wPageIndex = 0;
						}
						wDataIndex = 0;
					}
					
					//
					if(next_datalen)
					{
						memcpy(&OtaSlFlashBuf[wPageIndex][wDataIndex],&OtaRxPack[10+(rev_datalen-next_datalen)],next_datalen);
						wDataIndex += next_datalen;
						dwTotalRevLen += next_datalen;
					}
					
					if(dwTotalRevLen == RxFotaHeader.img_size && wDataIndex != 0)// RxHdr.seq == MaxSeq)		// 最后1包数据
					{
						OtaSlWriteFlash(wPageIndex);
						if(wPageIndex == 0)
						{
							wPageIndex = 1;
						}
						else
						{
							wPageIndex = 0;
						}
						wDataIndex = 0;
					}
					else if(dwTotalRevLen > RxFotaHeader.img_size)//RxHdr.seq > MaxSeq)			// 传输量超出
					{
						OtaSlAbortCmd(FAIL_TRANSPORT_ERROR);
						PreSeq = 0;
						//MaxSeq = 0;
//						OtaSlProcess = 0;
						OtaSlVarInit();
						return;
					}
				}else  //序列不对
				{
				
				}
			}
		}
		else if(RxHdr.typecmd == 0x02) //提交升级
		{
			HwClrWdt();
			u32 crc32_calc = crc32(SL_OTA_FLASH_BASE_ADDR, RxFotaHeader.img_size);

			if(crc32_calc != RxFotaHeader.file_crc32)
			{
				OtaSlAbortCmd(FAIL_IMAGE_ERROR);
				return;
			}
			else
			{
				HwClrWdt();
				u16 crc_calc = CheckSum_CheckMemory_X(SL_OTA_FLASH_BASE_ADDR, RxFotaHeader.img_size);
				
				DataFlashPowerDownSave.BootMark = FLASH_BOOT_BWK;
				DataFlashPowerDownSave.UpgradeLen = RxFotaHeader.img_size;
				DataFlashPowerDownSave.UpgradeCrc = crc_calc;	
				
				HwClrWdt();
				//升级文件烧录成功，则可以重启切换
				BootLoaderActive();
			}
		}
		else if(RxHdr.typecmd == 0x03)					// -->中止升级
		{
			PreSeq = 0;
			//MaxSeq = 0;
//			OtaSlProcess = 0;
			OtaSlVarInit();
		}

		TxHdr.fhdr1 = SL_OTA_HEAD1;
		TxHdr.fhdr2 = SL_OTA_HEAD2;
		TxHdr.sender = RxHdr.rcver;
		TxHdr.rcver = RxHdr.sender;
		TxHdr.type = RxHdr.type;
		TxHdr.typecmd = RxHdr.typecmd;
		TxHdr.len = 0x0001;
		TxHdr.seq = 0x0001;
		
		memcpy(&OtaTxPack[0], &TxHdr.fhdr1, sizeof(SlProtocolExtHdr));
		OtaTxPack[10] = 0x00;							// ACK:默认发送成功
		
		OtaSlSendManage();
	}
	else
	{
		//
	}
}



/*****************************************************************************
 函 数 名  : OtaSlReceiveProcess
 功能描述  : 松灵CAN-OTA升级接收数据解析
 输入参数  : void  
 返 回 值  : void
*****************************************************************************/
void OtaSlReceiveProcess(CanRxMessage RxMsg)
{
	u16 CheckSum1,CheckSum2;
	
	static u8 PackIndex = 0;
	static u8 DataCnt = 0;
	static u16 FrameDataLen = 0;
	
	u8 CanId;
	
	if(RxMsg.StdId == SL_OTA_PC_StdId1 || RxMsg.StdId == SL_OTA_PC_StdId2)
	{
		if(PackIndex == 0)														// 第一帧数据(获取包头&数据长度)
		{
			CanId = MultiCanAddr();
			if(RxMsg.Data[0] == SL_OTA_HEAD1 && RxMsg.Data[1] == SL_OTA_HEAD2 && RxMsg.Data[2] == ((SL_OTA_PC1_Addr & 0xF0) | CanId))	
			{		
				if(RxMsg.Data[3] == ((SL_OTA_BMS1_Addr & 0xF0) | CanId))							// 接收方ID正确(多包时需要区分)
				{
					PackIndex = 1;
					memset(OtaRxPack, 0, sizeof(OtaRxPack));
					memcpy(&OtaRxPack[0], &RxMsg.Data[0], 8);
					DataCnt = 1;
					FrameDataLen = (RxMsg.Data[6] | (RxMsg.Data[7] << 8)) & 0xFFFF;
					FrameDataLen += 12;
				}
				else
				{
					PackIndex = 0;
				}
				
				SlUpgradeTimer = APP_UPGRADE_MAX_TIME;							// CAN-OTA升级通讯超时计时器
			}
		}
		else 
		{
			if(DataCnt*8 > FrameDataLen) 
			{
				
				// 数据溢出,不处理
				OtaSlAbortCmd(FAIL_WRITE_IMAGE_ERROR);
			}else
			{	
				memcpy(&OtaRxPack[0+DataCnt*8], &RxMsg.Data[0], 8);					// 拷贝固件数据:8个
				DataCnt++;
				PackIndex++;
				
				if(DataCnt*8 >= FrameDataLen)
				{
					CheckSum1 = BYTES_TO_INT(RxMsg.Data[FrameDataLen-(DataCnt*8-8)-1],RxMsg.Data[FrameDataLen-(DataCnt*8-8)-2]);				// Crc16-Modbus
					CheckSum2 = ModbusCrc16(&OtaRxPack[0], FrameDataLen-2);							// 整包数据
					
					if(CheckSum1 == CheckSum2)											// 比较接收数据CRC是否正确
					{	
						OtaSlReceiveDecode();
					}
					else
					{
						
						//===> 失败的答复ACK
						OtaSlAbortCmd(FAIL_UNKNOWN);
					}
					DataCnt = 0;
					PackIndex = 0;
				}
			}
		}
	}
}

void OtaSlTimeOut(void)
{
	if(BMS_IS_UPGRADING())
	{
		if(SlUpgradeTimer == 1)
		{
			OtaSlAbortCmd(FAIL_CAN_COMMUINCATION_ERROR);
			SlUpgradeTimer = 0;
			OtaSlVarInit();
		}
		else if(SlUpgradeTimer > 0)
		{
			SlUpgradeTimer--;
		}
	}
}

void OtaSlJudge(void)
{
	if(!TimerOtaSlFlag)
	{
		return;
	}
	TimerOtaSlFlag = 0;
	
	if(BatStatus.Bits.PowerOff == 1) //关机不处理发送CAN数据
		return;
		
	OtaSlTimeOut();
}






