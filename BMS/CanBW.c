/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : Uart.c
  版 本 号   : 初稿
  作    者   : HuangBingQuan
  生成日期   : 2016年11月30日 星期三
  最近修改   :
  功能描述   : Uart通信
  函数列表   :
  修改历史   :
  1.日    期   : 2016年11月30日 星期三
    作    者   : HuangBingQuan
    修改内容   : 创建文件

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "CanBW.h"
#include "user.h"
#include "dataflash.h"
#include "Key.h"
#include "soc.h"
#include "BatterCheck.h"
#include "Calib.h"
//#include "Rtc.h"
#ifdef AFE_MODEULE
#include "AfeModule.h"
#endif
#include "PowerCtrl.h"



#define FLASH_CRC_READ

#define BW_PAYLOAD_LEN					6
typedef enum
{
	BW_ACK_OK = 0,
	BW_ACK_FAIL = 1,
	BW_ACK_ERR = 2,
}BW_ACK_E;

queue  BwTxQueue;
static CanRxMessage	RxMsgNow;
static CanTxMessage	TxMsgNow;
static u8 FunCode;
static u8 CmdCode;
static u8 WriteFlag;
static u8 MultiMsg;



/*****************************************************************************
 函 数 名  : BwMemset
 功能描述  : memset函数的自封装
 输入参数  : u8 *pDest
             u8 nC
             u16 nSize
 返 回 值  :
*****************************************************************************/
__attribute__((unused)) static void BwMemset(u8 *pDest, u8 nC, u16 Size)
{
    u16 i;

    for(i = 0; i < Size; i++)
    {
        *(pDest + i) = nC;
    }
}

/*****************************************************************************
 函 数 名  : ReadFlashOneByte
 功能描述  : Flash 数据读取1Byte
 输入参数  :
 返 回 值  :
*****************************************************************************/
static u8 ReadFlashOneByte(u16 u16_addr)
{
    u8 Dat;

    Dat = *(u8 *)(u16_addr);
    return Dat;
}

#ifdef FLASH_CRC_READ
/*****************************************************************************
 Prototype    : Crc16Calc
 Description  : 1
 Input        : u8 *data_arr
                u8 data_len
 Output       : None
 Return Value :
 Calls        :
 Called By    :
 CRC16-X-MODEM 是CRC16 的欧版标准，其多项式为：x16 + x12 + x5 + 1简记为0x1021，
 CRC16-X-MODEM 校验算法如下：
  History        :
  1.Date         : 2019/2/16
    Author       : Geek+
    Modification : Created function

*****************************************************************************/
static u16 Crc16Calc(u8 data_In, u16 PreCheckSum)
{
	u16 crc16;

	crc16 = PreCheckSum;
	crc16= (u16)((crc16>> 8) | (crc16<< 8));
	crc16^= data_In;
	crc16^= (u16)((crc16& 0xFF) >> 4);
	crc16^= (u16)((crc16<< 8) << 4);
	crc16^= (u16)(((crc16& 0xFF) << 4) << 1);

	return crc16;
}
#endif

/*****************************************************************************
 函 数 名  : FuncProduce
 功能描述  : Uart读写生产信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncProduce(void)
{
	u8 i, Ack;
	
    switch(CmdCode)                                              // 指令码
    {
        // 读取软件版本号
        case COMM_VERSION:
        {
        	if ((MultiMsg==0) && (RxMsgNow.Data[2]==0))
        	{
        		TxMsgNow.Data[0] = FunCode|(0<<1);
    			memcpy((u8 *)&TxMsgNow.Data[2], SW_VERSION, BW_PAYLOAD_LEN);
    			TxMsgNow.DLC = 8;   
    			queue_push_back(BwTxQueue, &TxMsgNow);
    			
    			TxMsgNow.Data[0] = FunCode|(1<<1);
        		memcpy((u8 *)&TxMsgNow.Data[2], SW_VERSION+BW_PAYLOAD_LEN, BW_PAYLOAD_LEN);
        		TxMsgNow.DLC = 8;
        		queue_push_back(BwTxQueue, &TxMsgNow);

        		TxMsgNow.Data[0] = FunCode|(2<<1);
        		memcpy((u8 *)&TxMsgNow.Data[2], SW_VERSION+BW_PAYLOAD_LEN*2, 1);
        		TxMsgNow.DLC = 3;
        		queue_push_back(BwTxQueue, &TxMsgNow);
        	}
        }
        break;

        /* 读取硬件版本号 */
		case COMM_HW_VER:
		{
			if ((MultiMsg==0) && (RxMsgNow.Data[2]==0))
        	{
        		TxMsgNow.Data[0] = FunCode|(0<<1);
    			memcpy((u8 *)&TxMsgNow.Data[2], HW_VERSION, BW_PAYLOAD_LEN);
    			TxMsgNow.DLC = 8;   
    			queue_push_back(BwTxQueue, &TxMsgNow);
    			
    			TxMsgNow.Data[0] = FunCode|(1<<1);
        		memcpy((u8 *)&TxMsgNow.Data[2], HW_VERSION+BW_PAYLOAD_LEN, BW_PAYLOAD_LEN);
        		TxMsgNow.DLC = 8;
        		queue_push_back(BwTxQueue, &TxMsgNow);
        	}
		}
		break ;

        //读取PCM二维码
        case COMM_TD_CODE:
        {
            if (WriteFlag == 1)
            {
            	Ack = BW_ACK_OK;
            	
                if (MultiMsg < 32/BW_PAYLOAD_LEN)
                {
                	i = RxMsgNow.DLC - 2;
                	if (i > BW_PAYLOAD_LEN)
                	{
                		i = BW_PAYLOAD_LEN;
                	}
                    memcpy(DataFlashAtOnceSave.QRCode+MultiMsg*6, (u8 *)&RxMsgNow.Data[2], i);  
                }
                else if (MultiMsg == 32/BW_PAYLOAD_LEN)
                {
                	DataFlashAtOnceSave.QRCode[30] = RxMsgNow.Data[2];
                	DataFlashAtOnceSave.QRCode[31] = RxMsgNow.Data[3];
                }
                else
                {
                	Ack = BW_ACK_ERR;
                }
                if (Ack == BW_ACK_OK)
                {
                	AtOnceSaveDataWrite();
                    AtOnceSaveDataRead();
                    TxMsgNow.Data[2] = BW_ACK_OK;
                    TxMsgNow.DLC = 3;
        			queue_push_back(BwTxQueue, &TxMsgNow);
                }
            }
            else if ((MultiMsg==0) && (RxMsgNow.Data[2]==0))
        	{
        		AtOnceSaveDataRead();

        		for (i=0; i<32/BW_PAYLOAD_LEN; i++)
        		{        		
        			TxMsgNow.Data[0] = FunCode|(i<<1);
    				memcpy((u8 *)&TxMsgNow.Data[2], DataFlashAtOnceSave.QRCode+i*6, BW_PAYLOAD_LEN);
    				TxMsgNow.DLC = 8;   
    				queue_push_back(BwTxQueue, &TxMsgNow);
    			}
    			TxMsgNow.Data[0] = FunCode|(5<<1);
        		TxMsgNow.Data[2] = DataFlashAtOnceSave.QRCode[30];
                TxMsgNow.Data[3] = DataFlashAtOnceSave.QRCode[31];
        		TxMsgNow.DLC = 4;
        		queue_push_back(BwTxQueue, &TxMsgNow);
        	}            
        }
        break;

        		
		//读取PACK二维码
		case COMM_SN:
		{
			if (WriteFlag == 1)
			{
				Ack = BW_ACK_OK;
				
				if (MultiMsg < 32/BW_PAYLOAD_LEN)
				{
					i = RxMsgNow.DLC - 2;
                	if (i > BW_PAYLOAD_LEN)
                	{
                		i = BW_PAYLOAD_LEN;
                	}
					memcpy(DataFlashAtOnceSave.SNCode+MultiMsg*6, (u8 *)&RxMsgNow.Data[2], i);  
				}
				else if (MultiMsg == 32/BW_PAYLOAD_LEN)
				{
					DataFlashAtOnceSave.SNCode[30] = RxMsgNow.Data[2];
					DataFlashAtOnceSave.SNCode[31] = RxMsgNow.Data[3];
				}
				else
				{
					Ack = BW_ACK_ERR;
				}
				if (Ack == BW_ACK_OK)
				{
					AtOnceSaveDataWrite();
					AtOnceSaveDataRead();
					TxMsgNow.Data[2] = BW_ACK_OK;
					TxMsgNow.DLC = 3;
					queue_push_back(BwTxQueue, &TxMsgNow);
				}
			}
			else if ((MultiMsg==0) && (RxMsgNow.Data[2]==0))
			{
				AtOnceSaveDataRead();

				for (i=0; i<32/BW_PAYLOAD_LEN; i++)
				{				
					TxMsgNow.Data[0] = FunCode|(i<<1);
					memcpy((u8 *)&TxMsgNow.Data[2], DataFlashAtOnceSave.SNCode+i*6, BW_PAYLOAD_LEN);
					TxMsgNow.DLC = 8;	
					queue_push_back(BwTxQueue, &TxMsgNow);
				}
				TxMsgNow.Data[0] = FunCode|(5<<1);
				TxMsgNow.Data[2] = DataFlashAtOnceSave.SNCode[30];
				TxMsgNow.Data[3] = DataFlashAtOnceSave.SNCode[31];
				TxMsgNow.DLC = 4;
				queue_push_back(BwTxQueue, &TxMsgNow);
			}			 
		}
		break;

        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncVoltage
 功能描述  : Uart读取电压信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncVoltage(void)
{
    u8 i;
//    u16 temp;

    switch(CmdCode)
    {
        // 读取总电芯电压值
      	case COMM_TOTAL_VOL:
        {            
            TxMsgNow.Data[2] = (u8)(BatData.VolTotal>>24);
            TxMsgNow.Data[3] = (u8)(BatData.VolTotal>>16);
            TxMsgNow.Data[4] = (u8)(BatData.VolTotal>>8);
            TxMsgNow.Data[5] = (u8)BatData.VolTotal;
            TxMsgNow.DLC = 6;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 读取全部各节电芯电压值
        case COMM_ALL_VOL:
        {
            for (i=0; i<CELL_NUM*2/BW_PAYLOAD_LEN; i++)
    		{        		
    			TxMsgNow.Data[0] = FunCode|(i<<1);
				TxMsgNow.Data[2] = (u8)(BatData.Bat[i*3+0].Vol >> 8);
                TxMsgNow.Data[3] = (u8)BatData.Bat[i*3+0].Vol;       
                TxMsgNow.Data[4] = (u8)(BatData.Bat[i*3+1].Vol >> 8);
                TxMsgNow.Data[5] = (u8)BatData.Bat[i*3+1].Vol;       
                TxMsgNow.Data[6] = (u8)(BatData.Bat[i*3+2].Vol >> 8);
                TxMsgNow.Data[7] = (u8)BatData.Bat[i*3+2].Vol;       
				TxMsgNow.DLC = 8;   
				queue_push_back(BwTxQueue, &TxMsgNow);
			}
            TxMsgNow.Data[0] = FunCode|(4<<1);
			TxMsgNow.Data[2] = (u8)(BatData.Bat[12].Vol >> 8);
            TxMsgNow.Data[3] = (u8)BatData.Bat[12].Vol;            
			TxMsgNow.DLC = 4;   
			queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 读取充电器电压值
        case COMM_CHG_VOL:
        {            
            TxMsgNow.Data[2] = (u8)(ChgAvgVol>>24);
            TxMsgNow.Data[3] = (u8)(ChgAvgVol>>16);
            TxMsgNow.Data[4] = (u8)(ChgAvgVol>>8);
            TxMsgNow.Data[5] = (u8)ChgAvgVol;
            TxMsgNow.DLC = 6;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

	#ifdef MCU_VDD_CHECK
        case COMM_VDD_VOL:
		{
			temp = BatReadVref2Vdd();
            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = (u8)(temp>>8);                                         //
            UartBuf[7] = (u8)(temp>>0);                                         //
        }
        break;
    #endif    
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncCurrent
 功能描述  : Uart读取电流信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncCurrent(void)
{
	u16 Data16;
	
    switch(CmdCode)
    {
        // 读取充电电流值
        case COMM_CHG_CUR:
        {       
        	Data16 = ChgCurAvgAD*OPA_CUR_GAIN;// =/4096*3300/25*1000;
            TxMsgNow.Data[2] = (u8)(ChgCurAvg>>8);
            TxMsgNow.Data[3] = (u8)ChgCurAvg;
            TxMsgNow.Data[4] = (u8)(Data16>>8);
            TxMsgNow.Data[5] = (u8)Data16;
            TxMsgNow.DLC = 6;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 读取马达1的电流值
        case COMM_DISCHG_CUR1:
        {            
            TxMsgNow.Data[2] = (u8)(DisCurAvg>>24);
            TxMsgNow.Data[3] = (u8)(DisCurAvg>>16);
            TxMsgNow.Data[4] = (u8)(DisCurAvg>>8);
            TxMsgNow.Data[5] = (u8)DisCurAvg;
            TxMsgNow.DLC = 6;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

	#ifdef AFE_MODEULE
        case COMM_AFE_BIAS:
        {            
            TxMsgNow.Data[2] = (u8)(DataFlashAtOnceSave.AfeMCBias>>24);
            TxMsgNow.Data[3] = (u8)(DataFlashAtOnceSave.AfeMCBias>>16);
            TxMsgNow.Data[4] = (u8)(DataFlashAtOnceSave.AfeMCBias>>8);
            TxMsgNow.Data[5] = (u8)DataFlashAtOnceSave.AfeMCBias;
            TxMsgNow.DLC = 6;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;
	#endif
	
        default:
            break;
    }
}

#if 0
#define TABLE_MAX		6

/* NTC阻值转化为AD值，须调整对应AD分辨率及分压阻值 */
#define NTC_AD(R)		(u16)((u32)R*(u32)ADBIT/(u32)(R+4700))

/* K氏温度表，单位0.1K，摄氏温度 T = TempK - 2731。分别对应80、60、40、20、0、-20℃的K氏温度 */
const u16 TempKTable[TABLE_MAX] = 
{
	(80*10 + 2731),	(60*10 + 2731),	(40*10 + 2731),	(20*10 + 2731),	(0*10 + 2731),	(-20*10 + 2731),
};

/* NTC对应阻值表，输入不同温度点下对应的阻值。分别对应80、60、40、20、0、-20℃的NTC阻值 */
const u16 AdcNtcTable[TABLE_MAX] = 
{
	NTC_AD(1251),	NTC_AD(2477),	NTC_AD(5310),	NTC_AD(12499),	NTC_AD(31813),	NTC_AD(92983),
};
/*****************************************************************************
 函 数 名  : AdcNtcToTempK
 功能描述  : NTC温度换算函数，查表法
 输入参数  : 
 			u16 AdcNtc: NTC的AD值 
 返 回 值  : 
 			u16，K氏温度返回值，0.1K
*****************************************************************************/
u16 AdcNtcToTempK(u16 AdcNtc)
{
	u8 i;
	u16 ad_diff, temp_dif, temp_k;

	if (AdcNtc <= AdcNtcTable[0])
		temp_k = TempKTable[0];	
	else
	{
		for (i = 1; i < TABLE_MAX; i++)
		{
			if(AdcNtc <= AdcNtcTable[i])
				break;
		}

		if (i == TABLE_MAX)
			temp_k = TempKTable[TABLE_MAX-1];
		else
		{
			ad_diff = AdcNtc - AdcNtcTable[i-1];
			temp_dif = (u16)((u32)ad_diff*(u32)(TempKTable[i-1] - TempKTable[i])/(u32)(AdcNtcTable[i] - AdcNtcTable[i-1]));
			temp_k = (TempKTable[i-1] - temp_dif);
		}
	}
	return temp_k;
}
#endif

/*****************************************************************************
 函 数 名  : FuncNtc
 功能描述  : Uart读取NTC信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncNtc(void)
{
    switch(CmdCode)
    {
        // 读取PCM温度值
    #if 0
        case COMM_PCM_NTC:
        {
            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = (u8)(NtcAvg >> 8);                         // 数据1# PCM温度值高位
            UartBuf[7] = (u8)NtcAvg;                                // 数据2# PCM温度值低位
        }
        break;
	#endif
	
		case COMM_PCM_NTC_K:
		{            
            TxMsgNow.Data[2] = (u8)(BatNtcTemp[0]/10-(273-50));
            TxMsgNow.Data[3] = (u8)(BatNtcTemp[1]/10-(273-50));
			TxMsgNow.Data[4] = (u8)(BatNtcTemp[2]/10-(273-50));
        //	TxMsgNow.Data[5] = (u8)(BatNtcTemp[3]/10-(273-50));
            TxMsgNow.DLC = 5;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
		break;

		case COMM_MOS_NTC_K:
		{            
            TxMsgNow.Data[2] = (u8)(MosNtcTemp[0]/10-(273-50));
            TxMsgNow.Data[3] = (u8)(MosNtcTemp[1]/10-(273-50));
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
		break;

        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncStatus
 功能描述  : Uart读取状态信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncStatus(void)
{
//	u16 Data16;
    switch(CmdCode)
    {
        // 读取系统当前的状态信息
        case COMM_STATUS:
        {
            TxMsgNow.Data[2] = BatStatus.Bytes.DisProt;
            TxMsgNow.Data[3] = BatStatus.Bytes.ChgProt;
            TxMsgNow.Data[4] = BatStatus.Bytes.HardFault;
            TxMsgNow.Data[5] = (BatStatus.Bytes.BmsState)>>8;
            TxMsgNow.Data[6] = (BatStatus.Bytes.BmsState)>>0;
            TxMsgNow.Data[7] = BatStatus.Bytes.InputState;
            TxMsgNow.DLC = 8;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 读取BMS额定容量
        case COMM_SET_CAP:
        {
            TxMsgNow.Data[2] = (u8)(DataFlashPowerDownSave.Soc.Fcc >> 8);
            TxMsgNow.Data[3] = (u8)DataFlashPowerDownSave.Soc.Fcc;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        /* 读取BMS剩余容量 */
        case COMM_NOW_CAP:
        {
            TxMsgNow.Data[2] = (u8)(DataFlashPowerDownSave.Soc.Rmc >> 8);
            TxMsgNow.Data[3] = (u8)DataFlashPowerDownSave.Soc.Rmc;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        /* 读取总放电次数 */
        case COMM_DIS_CNT:
        {
            TxMsgNow.Data[2] = (u8)(DataFlashPowerDownSave.Soc.Cycle>> 8);
            TxMsgNow.Data[3] = (u8)DataFlashPowerDownSave.Soc.Cycle;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        /* 读取SOC */
        case COMM_RATE_SOC:
        {
            TxMsgNow.Data[2] = DataFlashPowerDownSave.Soc.SocS;
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        /* 读取按键状态 */
        case COMM_KEY_PRESS:
        {
			#ifdef KEY_TO_LED
            TxMsgNow.Data[2] = KEY_LED_PRESS();
			#else
			TxMsgNow.Data[2] = 0;
			#endif
						TxMsgNow.Data[2] <<= 1;
            TxMsgNow.Data[2] |= CHG_IN();		// CHI state
            TxMsgNow.Data[2] <<= 1;
            TxMsgNow.Data[2] |= DET_IN();
						TxMsgNow.Data[3] = 0;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncCtrl
 功能描述  : Uart读取控制信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncCtrl(void)
{
    switch(CmdCode)
    {
        // 强制进入生产测试模式
        case COMM_SET_TEST_MODE:
        {
        	if ((RxMsgNow.Data[2]==0xA5) && (RxMsgNow.Data[3]==0x5A))
        	{
        		ENTER_FACTORY_TEST_MODE();
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else if ((RxMsgNow.Data[2]==0xA6) && (RxMsgNow.Data[3]==0x6A))
        	{
        		EXIT_FACTORY_TEST_MODE();
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);            
        }
        break;

        // 控制系统恢复出厂设置
        case COMM_RESET_FACTORY:
        {
            if ((RxMsgNow.Data[2]==0xA4) && (RxMsgNow.Data[3]==0x4A))
        	{
        		SocOcvTodo();
                   	
            	PowerDownSaveDataWrite();
            	
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        case COMM_CLEAR_STATUS:
        {
            if ((RxMsgNow.Data[2]==0xA3) && (RxMsgNow.Data[3]==0x3A))
        	{
        		BatStatus.Bytes.DisProt = 0;
        		BatStatus.Bytes.ChgProt = 0;
        		BatStatus.Bytes.HardFault = 0;
        		BatStatus.Bits.ShutDown = 0;
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        case COMM_RESET_SOC:
        {       	
        	if ((RxMsgNow.Data[2]==0xA2) && (RxMsgNow.Data[3]==0x2A))
        	{
        		SocOcvTodo();
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

	#ifdef AFE_MODEULE
        case COMM_AFEBIAS_RESET:
        {        	
            if ((RxMsgNow.Data[2]==0xA1) && (RxMsgNow.Data[3]==0x1A))
        	{
        		DataFlashAtOnceSave.AfeMCBias = 0;
        		DataFlashAtOnceSave.AfeSCBias = 0;
        		AtOnceSaveDataWrite();
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;
	#endif
	
        case COMM_SET_RESET:
        {
        	if ((RxMsgNow.Data[2]==0xA0) && (RxMsgNow.Data[3]==0x0A))
        	{
        		HwMcuReset();
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 控制系统关机
        case COMM_SET_POWER_OFF:
        {
        /*    if(BatStatus.Bits.ChgPlugin)                        // 充电器在时不执行关机
            {
                break;
            }	*/

            if ((RxMsgNow.Data[2]==0xB0) && (RxMsgNow.Data[3]==0x0B))
        	{
        		if (RxMsgNow.Data[4] > 0)
        		{
        			PowerShutDownTimerSet((u16)RxMsgNow.Data[4]*10);
        		}
        		else
        		{
        			PowerShutDownTimerSet(50);		// default 500ms
        		}
        		EXIT_FACTORY_TEST_MODE();
       			BatStatus.Bits.KeyOn = 0;
            	BatStatus.Bits.PowerOff = 1;
            	BatStatus.Bits.ShutDown = 1;
        		TxMsgNow.Data[2] = BW_ACK_OK;
        	}
        	else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        // 控制LED全亮
        case COMM_SET_LED_ON:
        {
        	if (IS_FACTORY_TEST_MODE())
        	{
	            LedTestSet = RxMsgNow.Data[2];
	            LedTestSet <<= 8;
	            LedTestSet |= RxMsgNow.Data[3];
	            TxMsgNow.Data[2] = BW_ACK_OK;
	        }
	        else
        	{
        		TxMsgNow.Data[2] = BW_ACK_FAIL;
        	}
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        case COMM_ENTER_SLEEP:
        {
        	BatStatus.Bits.Active = 0;
        	PowerEnterSleepTimerSet(ENTER_SLEEP_NORMAL_DELAY - 10U); // 延迟100ms进入休眠
        }
        break;


#if 1
        // 查询Flash地址[Start_Addr -> End_Addr]内容对应的CheckSum(CRC16-X-MODEM)
        // 示例：[F1 08 01 61 5A 04 00 20 00 B0 98 F2]  ==> Start_Addr:0x0020  End_Addr:0x00B0
        // 即：读取0x0020~0x00AF地址之间Flash数据的CheckSum(CRC16-X-MODEM)
        case COMM_READ_FLASH_CRC:                                   // 0x5A
        {
            u8 Temp_H, Temp_L, FlashReadData;
            u16 ReadAddr, StartAddr, EndAddr;
            u16 CrcCheckSum = 0;                                    // CRC16-X-MODEM

            Temp_H = RxMsgNow.Data[2];
            Temp_L = RxMsgNow.Data[3];
            StartAddr = BYTES_TO_INT(Temp_H, Temp_L);

            Temp_H = RxMsgNow.Data[4];
            Temp_L = RxMsgNow.Data[5];
            EndAddr = BYTES_TO_INT(Temp_H, Temp_L);

            for(ReadAddr = StartAddr; ReadAddr < EndAddr; ReadAddr++)
            {
                HwClrWdt();
                FlashReadData = ReadFlashOneByte(ReadAddr);
                CrcCheckSum = Crc16Calc(FlashReadData, CrcCheckSum);
            }

            TxMsgNow.Data[2] = (u8)(CrcCheckSum>> 8);
            TxMsgNow.Data[3] = (u8)CrcCheckSum;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;
#endif

        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncCalib(u8 RW_Flag)
 功能描述  : Uart读取/写入校准信息类指令
 输入参数  : 
 返 回 值  :
*****************************************************************************/
static void FuncCalib(void)
{
//    u8 i;
//	u16 Temp_K;
//  u32 Temp_Cur;
	RTC_TIME_S RtcTime;

    switch(CmdCode)
    {
    #ifndef AFE_MODEULE
        case COMM_CALIB_CELL_VOL:
        {        	
            UartBuf[6] = CalibBat(); 				// 校准成功/失败Flag 1：校准成功，0：校准失败    	
            UartBuf[6] &= CalibInit();
        #ifdef MCU_VDD_CHECK
            if (UartBuf[6] == 1)					// 电压校准成功则执行VDD校准
            {            
            	BatVddCalib();            
        		UartBuf[6] &= BatVddVerify();        	
            }
        #endif
            
            UartBuf[5] = 1;
        }
        break;
	#endif
	
        case COMM_CALIB_CHG_CURRENT:
        {
        	CalibChgCur((u8 *)(&RxMsgNow.Data[2]));
            if (CalibFlag == 1)  
            {
            	TxMsgNow.Data[2] = BW_ACK_OK;
            }
            else
            {
            	TxMsgNow.Data[2] = BW_ACK_FAIL;
            }
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);  
        }
        break;

        case COMM_CALIB_DIS_CURRENT:
        {
            CalibDsgCur((u8 *)(&RxMsgNow.Data[2]));
            if (CalibFlag == 1)  
            {
            	TxMsgNow.Data[2] = BW_ACK_OK;
            }
            else
            {
            	TxMsgNow.Data[2] = BW_ACK_FAIL;
            }
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);        
        }
        break;

        case COMM_CALIB_ZERO_CURRENT:
        {
        	if (IS_FACTORY_TEST_MODE())
        	{
        		CalibZeroCurrent();
        	}
        	else
        	{
        		CalibFlag = 0;
        	}
        	if (CalibFlag == 1)  
            {
            	TxMsgNow.Data[2] = BW_ACK_OK;
            }
            else
            {
            	TxMsgNow.Data[2] = BW_ACK_FAIL;
            }
            TxMsgNow.DLC = 3;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        case COMM_CALIB_CHARGER_VOL:
        {
        //	CalibChargerVol((u8 *)(&UartBuf[6]));
        }
        break;

	#ifndef AFE_MODEULE
        case COMM_K_CELL_VOL:                                       // No Use
        {
            if(RW_Flag == 0)                                        // 读/写操作:  '0'-写 '1'-读
            {
                for(i = 0; i < CELL_NUM; i++)
                {
                    DataFlashAtOnceSave.BAT_K[i] = ((UartBuf[6 + i * 2] << 8) | UartBuf[7 + i * 2]);
                }

                AtOnceSaveDataWrite();
            }

            AtOnceSaveDataRead();
            UartBuf[5] = 2 * CELL_NUM;

            for(i = 0; i < CELL_NUM; i++)
            {
                UartBuf[6 + i * 2] = (u8)(DataFlashAtOnceSave.BAT_K[i] >> 8);
                UartBuf[7 + i * 2] = (u8)(DataFlashAtOnceSave.BAT_K[i] & 0x00FF);

            }
        }
        break;
    #endif

        case COMM_K_CHG_CURRENT:                                    // No Use
        {
            TxMsgNow.Data[2] = (u8)(DATA_CUR_K_CHG>> 8);
            TxMsgNow.Data[3] = (u8)DATA_CUR_K_CHG;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

        case COMM_K_DIS_CURRENT:                                    // No Use
        {
            TxMsgNow.Data[2] = (u8)(DATA_CUR_K_DSGM>> 8);
            TxMsgNow.Data[3] = (u8)DATA_CUR_K_DSGM;
            TxMsgNow.DLC = 4;
            queue_push_back(BwTxQueue, &TxMsgNow);
        }
        break;

     	case COMM_CALIB_RTC:
        {
        	if (WriteFlag == 1)
        	{
	        	TxMsgNow.Data[2] = BW_ACK_FAIL;
	            if ((RxMsgNow.Data[2]>23) && (RxMsgNow.Data[2]<43))
	            {
	            	RtcTime.year = RxMsgNow.Data[2] + 2000;
	            	RtcTime.month = RxMsgNow.Data[3]&0x0F;
	            	RtcTime.week = (RxMsgNow.Data[3]>>4)&0x0F;
	            	RtcTime.day = RxMsgNow.Data[4];
	            	RtcTime.hour = RxMsgNow.Data[5];
	            	RtcTime.minute = RxMsgNow.Data[6];
	            	RtcTime.second = RxMsgNow.Data[7];
	            	
	            	RtcTimeSet(&RtcTime);
	            	TxMsgNow.Data[2] = BW_ACK_OK;
	            }
	            TxMsgNow.DLC = 3;
	        }
	        else
	        {
	        	RtcTimeRead(&RtcTime);
	        	TxMsgNow.Data[2] = RtcTime.year - 2000;
	        	TxMsgNow.Data[3] = RtcTime.month;
						TxMsgNow.Data[3] |= (RtcTime.week<<4);
	        	TxMsgNow.Data[4] = RtcTime.day;
	        	TxMsgNow.Data[5] = RtcTime.hour;
	        	TxMsgNow.Data[6] = RtcTime.minute;
	        	TxMsgNow.Data[7] = RtcTime.second;
	        	TxMsgNow.DLC = 8;
	        }
            queue_push_back(BwTxQueue, &TxMsgNow);        
        }
        break;
        
	#ifdef MCU_VDD_CHECK
        case COMM_CALIB_PCB_VDD:  
		{
			if(RW_Flag == 0)
			{
	 			BatVddCalib();
	        	UartBuf[6] = BatVddVerify();
	            UartBuf[5] = 1;
	        }
	        else
	        {
	        	UartBuf[5] = 4;
	        	UartBuf[6] = (u8)(VDD_CALIB_DATA>>8);
	        	UartBuf[7] = (u8)(VDD_CALIB_DATA>>0);
	        	UartBuf[8] = (u8)(VDD_VERIFY_DATA>>8);
	        	UartBuf[9] = (u8)(VDD_VERIFY_DATA>>0);
	        }
		}
		break;
	#endif
	
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : FuncSetting
 功能描述  : Uart读取配置信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
static void FuncSetting(void)
{
    switch(CmdCode)
    {

        default:
            break;
    }
}

void BwCanBmsRegister(queue TxQueue)
{
	BwTxQueue = TxQueue;
	
	TxMsgNow.ExtId = BW_ID_EXT_BMS;
	TxMsgNow.StdId = 0;
	TxMsgNow.IDE = CAN_ID_EXT;
	TxMsgNow.RTR = CAN_RTRQ_DATA;
}

void BwCanBmsReceiveProcess(CanRxMessage RxMsg)
{
	if (RxMsg.DLC < 2)
	{
		return;
	}

	RxMsgNow = RxMsg;	
	FunCode = RxMsgNow.Data[0]&0xF0;
	WriteFlag = RxMsgNow.Data[0]&0x01;
	MultiMsg = (RxMsgNow.Data[0]>>1)&0x07;
	CmdCode = RxMsgNow.Data[1];

	TxMsgNow.Data[0] = RxMsgNow.Data[0];
	TxMsgNow.Data[1] = RxMsgNow.Data[1];
	
	switch(FunCode)                                                 // 功能码判断
    {
        case FUNC_PRODUCE:                                          // 0x10 生产信息类
            FuncProduce();
            break;

        case FUNC_VOLTAGE:                                          // 0x20 电压信息类
            FuncVoltage();
            break;

        case FUNC_CURRENT:                                          // 0x30 电流信息类
            FuncCurrent();
            break;

        case FUNC_NTC:                                              // 0x40 温度信息类
            FuncNtc();
            break;

        case FUNC_STATUS:                                           // 0x50 状态信息类
            FuncStatus();
            break;

        case FUNC_CTRL:                                             // 0x60 控制管理类
            FuncCtrl();
            break;

        case FUNC_CALID:                                            // 0x70 校准管理类
            FuncCalib();
            break;

        case FUNC_SETTING:                                          // 0x80 配置管理类
            FuncSetting();
            break;
	#if 0
        case FUNC_ATE_INFO:
			FuncATE(flag);
			break;
	#endif
        default:
            break;
    }
}

