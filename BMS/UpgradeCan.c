#include <stdio.h>
#include <string.h>

#include "McuHal.h"
#include "AfeModule.h"
#include "DataFlash.h"
#include "gotoboot.h"


/* ---------------------------------------------CAN扩展帧ID------------------------------------------- */
/* OTA升级扩展帧ID */
#define UPDATA_DATA_EID      0x05020000
#define UPDATA_REQUEST_EID   0x04028000
#define UPDATA_ACK_EID       0x04200000

/*-------------------------------------------CAN升级协议结构体------------------------------------------*/

#pragma pack(1)	           // 按1字节对齐
typedef struct
{
	unsigned char order;   //0x10 请求发送多帧数据  0x11 allow   0x13  rec success
	unsigned short framelen;
	unsigned char num;
	unsigned char nc;
	unsigned char frameid[3];
}MultiFrameAllowstr;         //多帧数据允许发送及接收成功  from control  0x04028000

typedef struct 
{
	unsigned char number;  //序号
	unsigned char buf[7];  //数据内容
}BatteryMultiFrameDatastr;  //多帧数据格式    to control  0x05020000

typedef struct
{
	unsigned long len;
	unsigned short cs;
	unsigned long ver;
	unsigned char mark;
	unsigned char nc[3];
}FileMessagestr;
typedef struct
{
	unsigned long addr;
	unsigned long len;
	unsigned char cs;
	unsigned char buf[300];
}FileDatastr;
typedef struct 
{
	unsigned long len;
	unsigned short cs;
}ReceiveFileMessagestr;
typedef struct
{
	unsigned char mark;
	unsigned char block;
	unsigned long addr;
}FlashFilestr;

#pragma pack()						//  取消按1字节对齐

#define MARK_REQ_UPGRADE			0x5A
#define MARK_CFN_UPGRADE			0xA5

typedef enum
{
    FAILED = 0,
    PASSED = !FAILED
} Status;


#define FILE_DATA_START_ADDR	(BOOT_LOADER_SIZE)
#define FILE_DATA_END_ADDR		(BOOT_LOADER_SIZE+UPDATE_OFFEST_ADDR)

#define FLASH_BUF_LEN       	FLASH_PAGE_SIZE			// 一次写flash的字节数

u8 canTxrqFlag;
CanTxMessage CAN_TxMessage;
u32 UpdateBaseAddr;
MultiFrameAllowstr  MultiFrameAsk;
FileMessagestr  FileMessage;
FileDatastr  FileData;
ReceiveFileMessagestr  ReceiveFileMessage;
FlashFilestr FlashFile;
u32 Ram2FlashBuf[FLASH_BUF_LEN];



uint16_t CheckSum_CheckMemory(uint32_t checkAddr, uint32_t length)
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

/*****************************************************************************
 函 数 名  : WriteSector
 功能描述  : 写flash数据
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void FLSMAIN(u8 block)
{
//    uint32_t writeBuf[FLASH_PAGE_SIZE/4];
    uint32_t addr;  

//    memcpy((uint8_t *)writeBuf, Ram2FlashBuf, FLASH_BUF_LEN);
    addr = block * FLASH_BUF_LEN + FLASH_BASE_ADDR + UPDATE_OFFEST_ADDR;
  
    HwDataUnlock();
    HwDataPageWriteRandom(addr&0x0801FC00, Ram2FlashBuf, FLASH_BUF_LEN/4);
    HwDataLock();
}

/*****************************************************************************
 函 数 名  : DataJudge
 功能描述  : flash升级处理
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
void DataJudge(unsigned char mark)
{
	if(FlashFile.mark != 0x5a)
	{
		FlashFile.block = (unsigned char)((unsigned long)FileData.addr/FLASH_BUF_LEN);
		FlashFile.mark = 0x5a;
	}
	
	if(FlashFile.mark == 0x5a)
	{
		if( (FlashFile.block == FileData.addr/FLASH_BUF_LEN)
			&& (FlashFile.block == (FileData.addr+FileData.len-1)/FLASH_BUF_LEN) )
		{
			if(mark)
			{
				if(	FlashFile.addr == FileData.addr )
				{
					// FLSMAIN(SAM_SET_OPTION,0,FlashFile.block);  //升级成功重启
					FlashFile.mark  = 0;
				}
				else
				{
					if(FlashFile.addr != 0xffffff)  //上帧正好跨页，则不需拷贝，直接升级
					{
						FlashFile.addr = FileData.addr ;
						memcpy((u8 *)Ram2FlashBuf+FileData.addr%FLASH_BUF_LEN, FileData.buf, FileData.len-1);
					}
					FLSMAIN(FlashFile.block); //升级成功重启
					FlashFile.mark  = 0;
				}
			}
			else
			{
				memcpy((u8 *)Ram2FlashBuf+FileData.addr%FLASH_BUF_LEN, FileData.buf, FileData.len-1);
			}
		}
		else  //正好1k或跨1k
		{
			if( (FlashFile.block == FileData.addr/FLASH_BUF_LEN)
				&& (FlashFile.block != ((FileData.addr+FileData.len-1)/FLASH_BUF_LEN)) )
			{				
				memcpy((u8 *)Ram2FlashBuf+FileData.addr%FLASH_BUF_LEN,FileData.buf,(FLASH_BUF_LEN - FileData.addr%FLASH_BUF_LEN));				
				FLSMAIN(FlashFile.block);
				FlashFile.block = (FileData.addr+FileData.len-1)/FLASH_BUF_LEN;
				memcpy((u8 *)Ram2FlashBuf,FileData.buf+(FLASH_BUF_LEN-FileData.addr%FLASH_BUF_LEN),FileData.len-1-(FLASH_BUF_LEN-FileData.addr%FLASH_BUF_LEN));

				if(mark)
				{
					 FLSMAIN(FlashFile.block);
				}
				else
					FlashFile.addr = 0xffffff ;   //有跨区，则需特殊处理,特殊标记地址，下一步如果是直接启动升级，则不需拷贝
			}
			else  //换页，则先烧录再拷贝，
			{
				if(mark)
				{
					FLSMAIN(FlashFile.block);
				}
				else
				{
					FLSMAIN(FlashFile.block);
				}
				FlashFile.block = FileData.addr/FLASH_BUF_LEN;
				memcpy((u8 *)Ram2FlashBuf+FileData.addr%FLASH_BUF_LEN,FileData.buf,FileData.len-1);
			}
		}
	}
}

 /*****************************************************************************
 函 数 名  : MultiFrameAllowFun
 功能描述  : 应答请求多帧数据接收组包
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
 void MultiFrameAllowFun(void)
 {
	CAN_TxMessage.ExtId = UPDATA_ACK_EID;  //允许发送多帧数据报文
 	CAN_TxMessage.DLC = 8;
 	CAN_TxMessage.IDE     = CAN_ID_EXT;  
    CAN_TxMessage.RTR     = CAN_RTRQ_DATA;   

	CAN_TxMessage.Data[0] = 0x11;
	CAN_TxMessage.Data[1]= MultiFrameAsk.num;//包数
	CAN_TxMessage.Data[2]= 1;//起始包数
	CAN_TxMessage.Data[3]= 0xff;//nc
	CAN_TxMessage.Data[4]= 0xff;//nc
	CAN_TxMessage.Data[5]= MultiFrameAsk.frameid[0];
	CAN_TxMessage.Data[6]= MultiFrameAsk.frameid[1];
	CAN_TxMessage.Data[7]= MultiFrameAsk.frameid[2];  //请求地址
	
	canTxrqFlag = 1;
 }
 
 /*****************************************************************************
 函 数 名  : MultiFrameOkFun
 功能描述  : 应答多帧数据接收完成组包
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
 void MultiFrameOkFun(unsigned char err)
 {
	CAN_TxMessage.ExtId = UPDATA_ACK_EID;  //允许发送多帧数据报文
	CAN_TxMessage.DLC = 8;
 	CAN_TxMessage.IDE     = CAN_ID_EXT;  
    CAN_TxMessage.RTR     = CAN_RTRQ_DATA;  

	memcpy(CAN_TxMessage.Data, &MultiFrameAsk.order, 8);
	CAN_TxMessage.Data[0] = 0x13;
	CAN_TxMessage.Data[4] = err;

	canTxrqFlag = 1;
 }

/*****************************************************************************
 函 数 名  : AppUpgradeParse
 功能描述  : Can接收数据解析
 输入参数  :   
 返 回 值  : 
*****************************************************************************/
uint8_t AppUpgradeParse(CanRxMessage CanRxMsg, CanTxMessage *TxMsg)
{			       
    static unsigned long repeataddr = 0xffffff;
    unsigned char ERRCODE = 0XFF;
    unsigned short i;
    u16 crc_calc = 0;
    static u8 updateflag = 0;  //用于去获取升级的flash基地址

    switch(CanRxMsg.ExtId) 
    {
        /* can总线的程序升级数据处理 */
        case UPDATA_DATA_EID:   //多帧数据报文
        	/* 升级请求或确认处理，参数组编号=0x800000 */
            if(MultiFrameAsk.frameid[0]==0&&MultiFrameAsk.frameid[1]==0&&MultiFrameAsk.frameid[2]==0x80)  //TODO frameid 参数组编号赋值
            {
                if(CanRxMsg.Data[0]>0 && CanRxMsg.Data[0]<=MultiFrameAsk.num)//请求信息数据拷贝
                {
                    memcpy((unsigned char *)&FileMessage.len+(CanRxMsg.Data[0]-1)*7, CanRxMsg.Data+1, 7);
                }
                if(CanRxMsg.Data[0] == MultiFrameAsk.num)   //请求信息报文分析
                {
                    //判断是否接收成功 给ERRCODE赋值                    
                    if(FileMessage.mark == MARK_REQ_UPGRADE)  //升级数据请求则清除接收状态，准备接收
                    {
                        updateflag = 1;
                        ReceiveFileMessage.cs = 0;
                        ReceiveFileMessage.len = 0;
                        if(FileMessage.len > UPDATE_FILE_MAX_LEN)
                            ERRCODE &= 0XFD;					// 长度错误 bit1=0 
                        repeataddr = 0xffffffff;
                            
                    }
                    else if(FileMessage.mark == MARK_CFN_UPGRADE)  //升级文件发送完成处理逻辑
                    {
                        if(FileMessage.cs != ReceiveFileMessage.cs)
                            ERRCODE &= 0xfe;					// 校验错误 bit0=0 
                        if(FileMessage.len != ReceiveFileMessage.len)
                            ERRCODE &= 0XFD;					// 长度错误 bit1=0 
                        
                        if(ERRCODE==0xff)
                        {
                            DataJudge(0x5a);
                            /* 对整文件进行CRC校验 */
                            crc_calc = CheckSum_CheckMemory(FLASH_BASE_ADDR + UpdateBaseAddr + UPDATE_OFFEST_ADDR, FileMessage.len);
                            if(crc_calc != FileMessage.cs)  //校验码错误
                            {
                                ERRCODE &= 0xfe;				// 校验错误 bit0=0 
                            }
                            if (ERRCODE==0xff)
                            {
                                //升级文件烧录成功，则可以重启切换
                                BootLoaderActive();
                                DataFlashPowerDownSave.BootMark = FLASH_BOOT_BWK;
    							DataFlashPowerDownSave.UpgradeLen = FileMessage.len;
    							DataFlashPowerDownSave.UpgradeCrc = FileMessage.cs;
                            }
                        }
                    }
                    else 
                    	ERRCODE &= 0XFB;						// 地址错误 bit2=0
                    	
                    MultiFrameOkFun(ERRCODE);  //应答接收成功
                    memset((unsigned char *)&MultiFrameAsk, 0, 8);//接收完成，则清除多帧请求
                }
            }
            /* 多帧数据接收处理 */
            else
            {
                if(CanRxMsg.Data[0]>0 && CanRxMsg.Data[0]<=MultiFrameAsk.num)//数据拷贝
                {
                    memcpy((unsigned char *)&FileData.buf+(CanRxMsg.Data[0]-1)*7, CanRxMsg.Data+1, 7);
                    FileData.len += 7;
                }
                if((CanRxMsg.Data[0]==MultiFrameAsk.num) && FileData.len)	 //数据完整性判断
                {
                    //判断是否接收成功 给ERRCODE赋值
                    FileData.addr = MultiFrameAsk.frameid[2];
                    FileData.addr <<= 8;
                    FileData.addr += MultiFrameAsk.frameid[1];
                    FileData.addr <<= 8;
                    FileData.addr += MultiFrameAsk.frameid[0];
                    if(updateflag)
                    {
                        updateflag = 0;
                        UpdateBaseAddr = FileData.addr;
                    }
                    
                    if((FileData.len>=MultiFrameAsk.framelen) && (FileData.len<MultiFrameAsk.framelen+7))  //最后一帧可能不足7字节，修正
                    {
                        FileData.len = MultiFrameAsk.framelen;
                        //增加重复帧判断处理  
                        if(repeataddr != FileData.addr)  //重复帧则不处理
                        {
                            ReceiveFileMessage.len +=  FileData.len-1;
                            for(i=0; i<FileData.len-1; i++)
                            {
                                ReceiveFileMessage.cs += FileData.buf[i];
                                FileData.cs += FileData.buf[i];
                            }
                        }
                        //else
                        //	ERRCODE &= 0xfe;                        
                    }
                    else 
                        FileData.len = 1;
                        
                    if(FileData.cs != FileData.buf[FileData.len-1])
                        ERRCODE &= 0xfe;
                    if(FileData.len != MultiFrameAsk.framelen)
                        ERRCODE &= 0xfd;
                    
                    if(FileData.addr < 0x1000)		
                        ERRCODE &= 0xfb;						
                    else if(FileData.addr+FileData.len-1 > 0x20000)///1ffff)  //flash
                    {
                        //if(FileData.addr+FileData.len-1 >0xf2000||FileData.addr<0xf1000) //data flash
                        ERRCODE &= 0xfb;
                    }
                 /*
                    else if((FileData.addr+UPDATE_OFFEST_ADDR>=0x1e800 
                    		|| FileData.addr+(FileData.len-1)+UPDATE_OFFEST_ADDR >=0x1e800)
                    		&& FileData.addr+UPDATE_OFFEST_ADDR<0x1fc00)	//trimming data
                 */
                 	else if((FileData.addr>=FILE_DATA_END_ADDR || FileData.addr+(FileData.len-1)>=FILE_DATA_END_ADDR)
                    		&& FileData.addr<FILE_DATA_START_ADDR)	//trimming data
                    {
                        ERRCODE &= 0xfb;    
                    }
                    
                    MultiFrameOkFun(ERRCODE);  //应答接收成功	
                    
                    if(ERRCODE == 0xff)
                    {
                        //数据接收合法，则写入flash
                        //开辟1k缓冲区，连续地址满1k则写入
                        if(repeataddr != FileData.addr)
                        {
                             DataJudge(0); //数据接收处理

                        }
                        repeataddr = FileData.addr;
                    }
                    else 
                        repeataddr = 0xffffffff;
                    //DataJudge(0); //数据接收处理						
                    memset((unsigned char *)&MultiFrameAsk, 0, 8);//接收完成，则清除多帧请求
                }
            }
            break;
            
        case UPDATA_REQUEST_EID:   //远程升级命令启动帧请求或数据请求	       
            if(CanRxMsg.Data[0]==0x10 && CanRxMsg.Data[3]<40) //控制码  包数   
            {		
                memcpy((unsigned char *)&MultiFrameAsk,CanRxMsg.Data, 8);
                
                MultiFrameAllowFun();//应答允许多帧数据发送
                
                if( CanRxMsg.Data[1] == 12     
                    && CanRxMsg.Data[2] == 0   //长度
                    && CanRxMsg.Data[3] == 2    //包数
                    && CanRxMsg.Data[5] == 0
                    && CanRxMsg.Data[6] == 0
                    && CanRxMsg.Data[7] == 0x80)  //参数组编号
                {
                	//升级请求信息或升级完成信息请求帧
                    memset((unsigned char *)&FileMessage, 0, sizeof(FileMessage));
                }
                else  //升级文件请求,则把文件信息清除
                {
                    FileData.len = 0;
                    FileData.cs = 0;   
                    FileData.addr = 0;
                }
            }
            else   //数据不符合要求则清除多帧格式信息
                memset((unsigned char *)&MultiFrameAsk, 0, 8);
            break;
        
        default:
            break;
    }

    if(canTxrqFlag == 1)
    {    	
        *TxMsg = CAN_TxMessage;
        canTxrqFlag = 0;
        return 1;
    }

    return 0;
}


