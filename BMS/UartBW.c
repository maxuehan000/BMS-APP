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
#include "UartBW.h"
#include "user.h"
#include "dataflash.h"
#include "Key.h"
#include "soc.h"
#include "BatterCheck.h"
#include "Calib.h"
#ifdef AFE_MODEULE
#include "AfeModule.h"
#endif
#include "PowerCtrl.h"

//#define FLASH_CRC_READ

static volatile u8 UartBmsIntTxBufLen = 0;
static volatile u8 *UartBmsIntTxBuf = (void*)0;
volatile u8 UartLen;           // UART数据长度[模拟串口发送/接收]
volatile u8 Rx_UartLen;             // UART数据长度[实际处理串口数据]
volatile u8 Tx_UartLen;             // UART数据长度[实际处理串口数据]
#define BW_UART_BUF_LEN		160
volatile u8 UartBuf[BW_UART_BUF_LEN];            // UART数据缓存区
volatile u8 Rx_Flag;           // UART接收到有效数据头标识
volatile u8 Tx_Flag;           // UART接收到有效数据头标识
volatile u8 UartCmdActFlag;         // UART需处理指令标识
volatile u8 Uart_TimeoutCount;      // UART通信等待超时错误

/*****************************************************************************
 函 数 名  : BwMemset
 功能描述  : memset函数的自封装
 输入参数  : u8 *pDest
             u8 nC
             u16 nSize
 返 回 值  :
*****************************************************************************/
#if 0
static void BwMemset(u8 *pDest, u8 nC, u16 Size)
{
    u16 i;

    for(i = 0; i < Size; i++)
    {
        *(pDest + i) = nC;
    }
}
#endif
/*****************************************************************************
 函 数 名  : ReadFlashOneByte
 功能描述  : Flash 数据读取1Byte
 输入参数  :
 返 回 值  :
*****************************************************************************/
u8 ReadFlashOneByte(u16 u16_addr)
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
u16 Crc16Calc(u8 data_In, u16 PreCheckSum)
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

#ifdef UART_DEBUG
/*****************************************************************************
 函 数 名  : MyItoa
 功能描述  : itoa 自封装函数
 输入参数  : u8 *buf    输出数字值对应的字符串
             s32 value  输入数字值
             u8 radix   进制
 返 回 值  :
*****************************************************************************/
void MyItoa(u8 *buf, s32 value, u8 radix)
{
#if 1
#define LEN 16
    const u8 table[] = "0123456789ABCDEF";
    static XRAM u8 local[LEN + 1] = {0};
    u8 *p = &local[LEN];
    XRAM u8 sign = 0;
    XRAM u32 temp;

    if(radix < 2 || radix > 16)
    {
        *p = '\0';
        return;
    }

    if(value < 0 && radix == 10)
    {
        sign = 1;
        value = (u32)(0 - value);
    }

    temp = value;

    *p-- = '\0';

    do
    {
        *p-- = table[temp % (u32)radix];
        temp /= radix;
    }
    while(temp > 0);

    if(sign == 1)
    {
        *p-- = '-';
    }

    p++;

    while(*p)
    {
        (buf)[0] = *p++;
        ++(buf);
        UartLen++;
    }

#endif
}

/*****************************************************************************
 函 数 名  : UartDebugSendStr
 功能描述  : 调试时输出字符串
 输入参数  : u8 *Buf，uint8_t *tx_pData
 返 回 值  :
*****************************************************************************/
void UartDebugSendStr(u8 *Buf, u8 *tx_pData)
{
    XRAM u16 i, nLen;

    nLen = strlen((char const *)tx_pData);
    UartLen += nLen;

    for(i = 0; i < nLen; i++)
    {
        Buf[i] = tx_pData[i];
    }
}

/*****************************************************************************
 函 数 名  : UartDebugSendStrData
 功能描述  : 调试时数值转为字符串输出
 输入参数  : u8 *Buf,int32_t tx_data, int base
 返 回 值  :
*****************************************************************************/
void UartDebugSendStrData(u8 *Buf, int32_t tx_data, int base)
{
    if(base == 16)
        UartDebugSendStr(Buf, (u8 *)"0x");
    else if(base == 10)
        ;
    else if(base == 2)
        UartDebugSendStr(Buf, (u8 *)"0B");

    MyItoa((u8 *)&UartBuf[UartLen], tx_data, base);
}

/*****************************************************************************
 函 数 名  : UartSendStr
 功能描述  : 输出字符串
 输入参数  : u8 *tx_pData
 返 回 值  :
*****************************************************************************/
void UartSendStr(u8 *tx_pData)
{
    XRAM u16 i, nLen;

    nLen = strlen((char const *)tx_pData);
    UartLen = nLen;

    for(i = 0; i < nLen; i++)
    {
        UartBmsSendByte(tx_pData[i]);
    }
}

/*****************************************************************************
 函 数 名  : UartSendStrData
 功能描述  : 数值转为字符串输出
 输入参数  : s32 tx_data    数值
             int base       进制
 返 回 值  :
*****************************************************************************/
void UartSendStrData(s32 tx_data, int base)
{
    XRAM char buf[20] = {0};

    MyItoa((u8 *)buf, tx_data, base);

    if(base == 16)
        UartSendStr((u8 *)"0x");

    UartSendStr((u8 *)buf);

}
#endif

/*****************************************************************************
 函 数 名  : UartDataParse
 功能描述  : CheckSUM计算及结果比较
 输入参数  : void
 返 回 值  : 0: 成功    1: 失败
*****************************************************************************/
u8 UartDataParse(void)
{
    u8 i;
    u8 checksum;

    checksum = 0;

    for(i = 1; i < (Rx_UartLen - 2); i++)
    {
        checksum += UartBuf[i];
    }

    if(checksum != UartBuf[Rx_UartLen - 2])
    {
        return 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : UartReceive
 功能描述  : Uart接收中断入口
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void UartReceive(u8 rdata)
{
    u8 rx_data;
    static XRAM u8 Data_Len = 0;

    rx_data = rdata;                                                // 获取当前接收到数据

    if(UartCmdActFlag || Tx_Flag)
        return;

    if((rx_data == DATA_HEAD) && (Rx_Flag == 0))                    // 0xF1 帧头
    {
        Rx_Flag = 1;
        Rx_UartLen = 0;
        Data_Len = 0;
    }

    if(Rx_Flag)
    {
        Uart_TimeoutCount = UART_TIMEOUT;                           // 帧内数据时间间隔不高于20ms

        if(Rx_UartLen == (sizeof(UartBuf) - 1))
        {
            Rx_Flag = 0;
            Rx_UartLen = 0;
        }

        if(Rx_UartLen < 5)                                          // 发送地址/接收地址/功能码/指令码/数据长度
        {
            UartBuf[Rx_UartLen] = rx_data;
            Rx_UartLen++;
        }
        else if(Rx_UartLen == 5)                                    // 数据长度
        {
            UartBuf[Rx_UartLen] = rx_data;
            Data_Len = rx_data;                                     // 获取: 数据域Data 长度

            if(Data_Len+6 > sizeof(UartBuf))
            {
                Data_Len = 0;
                Rx_UartLen = 0;
                Rx_Flag = 0;
                return;
            }

            Rx_UartLen++;

        }
        else if(Rx_UartLen <= (5 + Data_Len))                       // 数据域Data
        {
            UartBuf[Rx_UartLen] = rx_data;
            Rx_UartLen++;
        }
        else if(Rx_UartLen == (6 + Data_Len))                       // Checksum校验码
        {
            UartBuf[Rx_UartLen] = rx_data;
            Rx_UartLen++;
        }
        else if(Rx_UartLen == (7 + Data_Len))                       // 0xF2帧尾
        {
            Rx_UartLen++;

            if(rx_data == DATA_END)
            {
                UartCmdActFlag = 1;                                 // Uart完整一串数据接收完成,待处理
                Rx_Flag = 0;
            }

            UartLen = Rx_UartLen;
        }
        else
        {
            if(!UartCmdActFlag)                                     // 防止正常接收数据完成后,未处理就被清除 Neiyang->2018.05.09
            {
                Rx_Flag = 0;
                Rx_UartLen = 0;
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : UartBmsStartSend
 功能描述  : 开始Uart发送
 输入参数  : void
 返 回 值  : void
*****************************************************************************/
void UartBmsStartSend(u8 *txbuf, u8 len)
{
	if (UartBmsIntTxBufLen)
		return;
		
	UartBmsRxDisable();
    Delay1ms(2);

    Tx_Flag = 1;
    
    UartBmsIntTxBufLen = len-1;
    UartBmsIntTxBuf = txbuf;
    UartBmsSendByteIT(*UartBmsIntTxBuf);
    Tx_UartLen = UartBmsIntTxBufLen;
}

/*****************************************************************************
 函 数 名  : UartBmsSend
 功能描述  : Uart发送
 输入参数  : void
 返 回 值  : void
*****************************************************************************/
void UartBmsSend(void)
{
    if(Tx_Flag)
    {
        if(UartBmsIntTxBufLen > 0)
        {
        	UartBmsIntTxBufLen--;
            UartBmsIntTxBuf++;
            UartBmsSendByteIT(*UartBmsIntTxBuf);
        }
        else
        {
            UartCmdActFlag  = 0;
            Tx_Flag = 0;
            Rx_Flag = 0;
            //Delay10us_INT(50);
						UartBmsRxEnable();
        }
        Tx_UartLen = UartBmsIntTxBufLen;
    }
}

/*****************************************************************************
 函 数 名  : FuncProduce
 功能描述  : Uart读写生产信息类指令
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
void FuncProduce(u8 RW_Flag)
{
    switch(UartBuf[4])                                              // 指令码
    {
        // 读取软件版本号
        case COMM_VERSION:
        {
            UartBuf[5] = sizeof(SW_VERSION) - 1;
            memcpy((u8 *)&UartBuf[6], SW_VERSION, UartBuf[5]);
        }
        break;

        /* 读取硬件版本号 */
		case COMM_HW_VER:
		{
			UartBuf[5] = sizeof(HW_VERSION) - 1;
			memcpy((u8 *)&UartBuf[6], HW_VERSION, UartBuf[5]);
		}
		break ;

        //读取二维码
        case COMM_TD_CODE:
        {
            if(RW_Flag == 0)
            {
								ZeroClib(1);
                if(UartBuf[5] <= 32)
                {
                    memcpy(DataFlashAtOnceSave.QRCode, (u8 *)&UartBuf[6], 32);
                    AtOnceSaveDataWrite();
                }
            }

            AtOnceSaveDataRead();
            UartBuf[5] = 32;
            memcpy((u8 *)&UartBuf[6], DataFlashAtOnceSave.QRCode, 32);
						
        }
        break;

        		
#if 1	// 沿用V13，使用SN指令写入PACK端的二维码
		case COMM_SN:
		{
			if (UartBuf[5] <= 20)
			{
	            if(RW_Flag == 0)
	            {
	                memcpy(DataFlashAtOnceSave.SNCode, (u8 *)&UartBuf[6], UartBuf[5]);
	                AtOnceSaveDataWrite();
	            }

				UartBuf[5] = 20;	// 统一回复20字节
	            AtOnceSaveDataRead();            
	            memcpy((u8 *)&UartBuf[6], DataFlashAtOnceSave.SNCode, 20);
	        }
	        else
	        {	        
            	UartBuf[5] = 1;
            	UartBuf[5] = 0;
            }
        }
		break;
#endif

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
void FuncVoltage(void)
{
    u8 i;
//    u16 temp;

    switch(UartBuf[4])
    {
        // 读取总电芯电压值
      	case COMM_TOTAL_VOL:
        {
            UartBuf[5] = 4;                                         // 数据长度 2个
            UartBuf[6] = (u8)(BatData.VolTotal>>24);                                         // 数据1# 总电压高位
            UartBuf[7] = (u8)(BatData.VolTotal>>16);                                         // 数据2# 总电压低位
            UartBuf[8] = (u8)(BatData.VolTotal>>8);               // 数据1# 总电压高位
            UartBuf[9] = (u8)BatData.VolTotal;                      // 数据2# 总电压低位
        }
        break;
#if 0
        // 读取单节电芯电压值
        case COMM_SINGLE_VOL:
        {
            i = UartBuf[6];                                         // 待查询单节电芯Number
            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = (u8)(BatData.Bat[i - 1].Vol >> 8);         // 数据1# 单节电芯电压高位
            UartBuf[7] = (u8)BatData.Bat[i - 1].Vol;                // 数据2# 单节电芯电压低位
        }
        break;
#endif

        // 读取全部各节电芯电压值
        case COMM_ALL_VOL:
        {
            UartBuf[5] = 2 * CELL_NUM;                              // 数据长度 2*N个

            for(i = 0; i < CELL_NUM; i++)
            {
                UartBuf[6 + i * 2] = (u8)(BatData.Bat[i].Vol >> 8); // 数据1x# 单节电芯电压高位
                UartBuf[7 + i * 2] = (u8)BatData.Bat[i].Vol;        // 数据2x# 单节电芯电压低位
            }
        }
        break;

        // 读取充电器电压值
        case COMM_CHG_VOL:
        {
            UartBuf[5] = 4;                                         // 数据长度 2个
            UartBuf[6] = (u8)(ChgAvgVol >> 24);                                         //
            UartBuf[7] = (u8)(ChgAvgVol >> 16);                                         //
            UartBuf[8] = (u8)(ChgAvgVol >> 8);                      // 数据1# 充电器总电压高位
            UartBuf[9] = (u8)ChgAvgVol;                              // 数据2# 充电器总电压低位
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
void FuncCurrent(void)
{
	u16 Data16;
	
    switch(UartBuf[4])
    {
        // 读取充电电流值
        case COMM_CHG_CUR:
        {
            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = (u8)(ChgCurAvg >> 8);                      // 数据1# 充电电流值高位
            UartBuf[7] = (u8)ChgCurAvg;                             // 数据2# 充电电流值低位
        }
        break;

        // 读取马达1的电流值
        case COMM_DISCHG_CUR1:
            UartBuf[5] = 4;                                         // 数据长度 2个
            UartBuf[6] = (u8)(DisCurAvg >> 24);               // 数据1# 马达1的电流值高位
            UartBuf[7] = (u8)(DisCurAvg >> 16);               // 数据2# 马达1的电流低位
            UartBuf[8] = (u8)(DisCurAvg >> 8);                // 数据1# 马达1的电流值高位
            UartBuf[9] = (u8)DisCurAvg;
            break;

        // 读取运放充电电流值
        case COMM_CHG_OPA_CUR:
            UartBuf[5] = 2;                                   // 数据长度 2个
            Data16 = (u32)ChgCurAvgAD*OPA_CUR_GAIN;					  // AD/4095*3300/25*1000
            UartBuf[6] = (u8)(Data16 >> 8);
            UartBuf[7] = (u8)Data16;
            break;

	#ifdef AFE_MODEULE
        case COMM_AFE_BIAS:
            UartBuf[5] = 4;                                         // 数据长度 4个
            UartBuf[6] = (u8)(DataFlashAtOnceSave.AfeMCBias >> 24);               // 数据1# 马达1的电流值高位
            UartBuf[7] = (u8)(DataFlashAtOnceSave.AfeMCBias >> 16);               // 数据2# 马达1的电流低位
            UartBuf[8] = (u8)(DataFlashAtOnceSave.AfeMCBias >> 8);               // 数据1# 马达1的电流值高位
            UartBuf[9] = (u8)(DataFlashAtOnceSave.AfeMCBias >> 0);               // 数据2# 马达1的电流低位
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
void FuncNtc(void)
{
    switch(UartBuf[4])
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
			UartBuf[5] = 10;
			for(u8 i=0; i<BAT_NTC_NUM; i++)
			{
				UartBuf[6+i*2] = (u8)(BatNtcTemp[i]>>8);
				UartBuf[7+i*2] = (u8)(BatNtcTemp[i]);
			}
			UartBuf[12] = (u8)(DmosNtcTemp>>8);
			UartBuf[13] = (u8)DmosNtcTemp;
			UartBuf[14] = (u8)(CmosNtcTemp>>8);
			UartBuf[15] = (u8)CmosNtcTemp;
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
void FuncStatus(void)
{
    switch(UartBuf[4])
    {
        // 读取系统当前的状态信息
        case COMM_STATUS:
        {
            UartBuf[5] = 7;                                         // 数据长度 6个
            UartBuf[6] = BatStatus.Bytes.DisProt;
            UartBuf[7] = BatStatus.Bytes.ChgProt;
            UartBuf[8] = BatStatus.Bytes.HardFault;
            UartBuf[9] = (u8)BatStatus.Bytes.BmsState;
            UartBuf[10] = (u8)(BatStatus.Bytes.BmsState>>8);
            UartBuf[11] = (u8)BatStatus.Bytes.InputState;
            UartBuf[12] = (u8)(BatStatus.Bytes.InputState>>8);
        }
        break;

        // 读取BMS额定容量
        case COMM_SET_CAP:
        {
            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = (u8)(DataFlashPowerDownSave.Soc.Fcc >> 8);                 // 数据1# BMS额定容量高位
            UartBuf[7] = (u8)DataFlashPowerDownSave.Soc.Fcc;                        // 数据2# BMS额定容量低位
        }
        break;

        /* 读取BMS剩余容量 */
        case COMM_NOW_CAP:
        {
            UartBuf[5] = 2;
            UartBuf[6] = (u8)(DataFlashPowerDownSave.Soc.Rmc >> 8);
            UartBuf[7] = (u8)DataFlashPowerDownSave.Soc.Rmc;
        }
        break;

        /* 读取总放电次数 */
        case COMM_DIS_CNT:
        {
            UartBuf[5] = 2;
            UartBuf[6] = (u8)(DataFlashPowerDownSave.Soc.Cycle >> 8);
            UartBuf[7] = (u8)DataFlashPowerDownSave.Soc.Cycle;
        }
        break;

        /* 读取SOC */
        case COMM_RATE_SOC:
        {
            UartBuf[5] = 1;
            UartBuf[6] = DataFlashPowerDownSave.Soc.SocS;
        }
        break;

        /* 读取按键状态 */
        case COMM_KEY_PRESS:
        {
            UartBuf[5] = 2;
        #ifdef KEY_TO_LED
            UartBuf[7] = KEY_LED_PRESS();
        #else
            UartBuf[7] = 0;
        #endif
            UartBuf[6] = VLOCK_IN();
        }
        break;

        // 读取放电短路保护状态
        case COMM_DIS_SHORT:
        {
            UartBuf[5] = 1;                                         // 数据长度 1个
        #ifndef AFE_MODEULE
            UartBuf[6] = SHORT_VALID();                             // 数据1#
        #else
        	UartBuf[6] = BatStatus.Bits.DisSC;
        #endif
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
void FuncCtrl(void)
{
    switch(UartBuf[4])
    {
        // 控制LED全亮
        case COMM_SET_LED_ON:
        {
            TestLedFlag = 1;
            UartBuf[5] = 0;                                         // 数据长度 0个
        }
        break;

        // 控制LED全灭
        case COMM_SET_LED_OFF:
        {
            TestLedFlag = 0;
            UartBuf[5] = 0;                                         // 数据长度 0个
        }
        break;

        // 强制进入生产测试模式
        case COMM_SET_TEST_MODE:
        {
            ENTER_FACTORY_TEST_MODE();
            TestLedFlag = 1;
		//	BatStatus.Bits.KeyOn = 1;
        }
        break;

        // 控制系统恢复出厂设置
        case COMM_RESET_FACTORY:
        {
        	TestLedFlag = 0;
            memset((u8 *)&DataFlashPowerDownSave, 0, sizeof(DataFlashPowerDownSave));
            DataFlashPowerDownSave.Soc.Fcc = CAPACITY_TYPE;         // 防SOC计算除0
            PowerDownSaveDataWrite();
            UartBuf[5] = 1;                                         // 数据长度 1个
            UartBuf[6] = 1;                                         // 数据1#
        }
        break;

        case COMM_CLEAR_STATUS:
        {
        	BatStatus.Bytes.DisProt = 0;
        	BatStatus.Bytes.ChgProt = 0;
        	BatStatus.Bytes.HardFault = 0;
        	BatStatus.Bits.ShutDown = 0;
        	UartBuf[5] = 1;                                         // 数据长度 1个
            UartBuf[6] = 1;                                         // 数据1#
        }
        break;

        case COMM_RESET_SOC:
        {
        	SocOcvTodo();        	
        	UartBuf[5] = 1;                                         // 数据长度 1个
            UartBuf[6] = 1;                                         // 数据1#
        }
        break;

	#ifdef AFE_MODEULE
        case COMM_AFEBIAS_RESET:
        {        	
        	DataFlashAtOnceSave.AfeMCBias = 0;
        	DataFlashAtOnceSave.AfeSCBias = 0;
        	AtOnceSaveDataWrite();
        	UartBuf[5] = 1;                                         // 数据长度 1个
            UartBuf[6] = 1;                                         // 数据1#
        }
        break;
	#endif
	
        case COMM_SET_RESET:
        {
        	HwMcuReset();											// no replay comm...
        }
        break;

        // 控制系统关机
        case COMM_SET_POWER_OFF:
        {
        /*    if(BatStatus.Bits.ChgPlugin)                        // 充电器在时不执行关机
            {
                break;
            }	*/
            if (UartBuf[5] == 1)
            {
            	PowerShutDownTimerSet((u16)UartBuf[6]*10);
            }
            else
            {
            	PowerShutDownTimerSet(50);		// default 500ms
            }
            UartBuf[5] = 0;                                         // 数据长度 0个
            EXIT_FACTORY_TEST_MODE();
       //	BatStatus.Bits.KeyOn = 0;
            BatStatus.Bits.PowerOff = 1;
            BatStatus.Bits.ShutDown = 1;
        }
        break;

        case COMM_ENTER_SLEEP:
        {
        	BatStatus.Bits.Active = 0;
        	PowerEnterSleepTimerSet(ENTER_SLEEP_NORMAL_DELAY - 10);		// 延迟100ms进入休眠
        }
        break;

        case COMM_SET_BLANCE:
		{
			if (IS_FACTORY_TEST_MODE())
			{			
				FactoryBalanceSelect = ((u32)(UartBuf[6]<<24)|UartBuf[7]<<16|UartBuf[8]<<8|UartBuf[9]);
				FactoryBalanceEnable = 1;
				UartBuf[6] = 1;
			}
			else
			{
				UartBuf[6] = 0;
			}
			UartBuf[5] = 1;
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

            Temp_H = UartBuf[6];
            Temp_L = UartBuf[7];
            StartAddr = BYTES_TO_INT(Temp_H, Temp_L);

            Temp_H = UartBuf[8];
            Temp_L = UartBuf[9];
            EndAddr = BYTES_TO_INT(Temp_H, Temp_L);

            for(ReadAddr = StartAddr; ReadAddr < EndAddr; ReadAddr++)
            {
                HwClrWdt();
                FlashReadData = ReadFlashOneByte(ReadAddr);
                CrcCheckSum = Crc16Calc(FlashReadData, CrcCheckSum);
            }

            UartBuf[5] = 2;                                         // 数据长度 2个
            UartBuf[6] = HI_BYTE(CrcCheckSum);                      // 数据1#
            UartBuf[7] = LO_BYTE(CrcCheckSum);                      // 数据2#
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
 输入参数  : RW_Flag 读/写操作标志
 返 回 值  :
*****************************************************************************/
void FuncCalib(u8 RW_Flag)
{
//    u8 i;
//	u16 Temp_K;
//    u32 Temp_Cur;

    switch(UartBuf[4])
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
        	CalibChgCur((u8 *)(&UartBuf[6]));
            UartBuf[5] = 1;                                         // 回复数据1个DATA   
            UartBuf[6] = CalibFlag;
        }
        break;

        case COMM_CALIB_DIS_CURRENT:
        {
            CalibDsgCur((u8 *)(&UartBuf[6]));
            UartBuf[5] = 1;                                         // 回复数据1个DATA   
            UartBuf[6] = CalibFlag;          
        }
        break;

        case COMM_CALIB_ZERO_CURRENT:
        {
        	CalibZeroCurrent();
        	UartBuf[5] = 1;                                         // 回复数据1个DATA   
            UartBuf[6] = CalibFlag;
        }
        break;

        case COMM_CALIB_CHARGER_VOL:
        {
        //	CalibChargerVol((u8 *)(&UartBuf[6]));
        	UartBuf[5] = 1;                                         // 回复数据1个DATA   
            UartBuf[6] = CalibFlag;
        }
        break;
			case COMM_CALIB_VPACK_VOL:  //校准VPACK电压
				{							
						CalibFlag = CalibPackVol((u8 *)(&UartBuf[6]));
						UartBuf[5] = 1;
					  UartBuf[6] = CalibFlag;
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
            if(RW_Flag == 0)
            {
                DATA_CUR_K_CHG = ((UartBuf[6] << 8) | UartBuf[7]);
                AtOnceSaveDataWrite();
            }

            AtOnceSaveDataRead();
            UartBuf[5] = 2;
            UartBuf[6] = (u8)(DATA_CUR_K_CHG >> 8);
            UartBuf[7] = (u8)(DATA_CUR_K_CHG & 0x00FF);
        }
        break;

        case COMM_K_DIS_CURRENT:                                    // No Use
        {
            if(RW_Flag == 0)
            {
                DATA_CUR_K_DSGM = ((UartBuf[6] << 8) | UartBuf[7]);
                AtOnceSaveDataWrite();
            }

            AtOnceSaveDataRead();
            UartBuf[5] = 2;
            UartBuf[6] = (u8)(DATA_CUR_K_DSGM >> 8);
            UartBuf[7] = (u8)(DATA_CUR_K_DSGM & 0x00FF);
        }
        break;

        case COMM_K_ZERO_CURRENT:
		{
            if(RW_Flag == 0)
            {
                DataFlashAtOnceSave.AfeMCBias = ((UartBuf[6] << 8) | UartBuf[7]);
                AtOnceSaveDataWrite();
            }

            AtOnceSaveDataRead();
            UartBuf[5] = 2;
            UartBuf[6] = (u8)(DataFlashAtOnceSave.AfeMCBias >> 8);
            UartBuf[7] = (u8)(DataFlashAtOnceSave.AfeMCBias & 0x00FF);
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
void FuncSetting(void)
{
    switch(UartBuf[4])
    {
        default:
            break;
    }
}

#if 1
typedef struct
{
	u8 Qr;
	u8 SoftVer;
	u8 HardVer;
	u8 Vol;
	u8 DisCur;
	u8 ChgCur;
	u8 Ntc;
	u8 Status;
	u8 SumLen;
}GENERAL_INFO_LEN_T;

#define GNR_INF_NUM			8
#define GNR_INF_QR_LEN		32
#define GNR_INF_SW_LEN		(sizeof(SW_VERSION)-1)
#define GNR_INF_HW_LEN		(sizeof(HW_VERSION)-1)
#define GNR_INF_VOL_LEN		(8+2*CELL_NUM)			// PackVol/10,ChargerVol/10,CellVol
#define GNR_INF_DCUR_LEN 	2						// DisCurAvg/10	
#define GNR_INF_CCUR_LEN	(2+2)					// ChgCurAvg/10,opa_cur
#define GNR_INF_NTC_LEN		((3+2)*2)				// 3CELL_NTC + 2MOS_NTC
#define GNR_INF_STA_LEN		(1+5+6)					// // 1KeyStatue+5BatStatus+6AFEStatus
#define GNR_INF_TOTAL_LEN	(GNR_INF_NUM+GNR_INF_QR_LEN+GNR_INF_SW_LEN+GNR_INF_HW_LEN+GNR_INF_VOL_LEN\
								+GNR_INF_DCUR_LEN+GNR_INF_CCUR_LEN+GNR_INF_NTC_LEN+GNR_INF_STA_LEN)
const GENERAL_INFO_LEN_T AteGeneralInfoLenth =
{
	.Qr = GNR_INF_QR_LEN,	
	.SoftVer = GNR_INF_SW_LEN,
	.HardVer = GNR_INF_HW_LEN,
	.Vol = GNR_INF_VOL_LEN,
	.DisCur = GNR_INF_DCUR_LEN,
	.ChgCur = GNR_INF_CCUR_LEN,
	.Ntc = GNR_INF_NTC_LEN,
	.Status = GNR_INF_STA_LEN,
	.SumLen = GNR_INF_TOTAL_LEN
};

void FuncATE(u8 rwFlag)
{
	#define ATE_FORCE_CTRL_DELAY		500
	u8 i = 0;
	u8 j = 0;
	u16 Data16;

	switch (UartBuf[4])
	{
		// 写入MOS,LED控制
		case COMM_ATE_MOS_LED_CTRL:
		{	
			if (IS_FACTORY_TEST_MODE() && UartBuf[5]>=4)
			{				
				// 测试模式下,临时控制MOS持续5S,5S后解除强制控制
				// UartBuf[6] bit0 放电MOS控制; =1为开启 =0为关闭
				// UartBuf[7] bit1 充电MOS控制; =1为开启 =0为关闭
				// UartBuf[8] bit0-3 LED1-4 / bit4 LED_T
				// UartBuf[9] bit7 valid / bit0-6 127S
				if (UartBuf[6] & 0x01)
				{
					BatStatus.Bits.Dis = 1;		// AfeFetCtrl() would do DmosON
				}
				else
				{
					BatStatus.Bits.Dis = 0;		// AfeFetCtrl() would do DmosOFF
				}

				if (UartBuf[7] & 0x01)
				{
					CHG_MOS2_ON();
					BatStatus.Bits.Chg = 1;		// AfeFetCtrl() would do CmosON
				}
				else
				{
					CHG_MOS2_OFF();
					BatStatus.Bits.Chg = 0;		// AfeFetCtrl() would do CmosON
				}
				
				if (UartBuf[8]&0x01)
				{
					LED_1_ON();
				}
				else
				{
					LED_1_OFF();
				}
				if (UartBuf[8]&0x02)
				{
					LED_2_ON();
				}
				else
				{
					LED_2_OFF();
				}
				if (UartBuf[8]&0x04)
				{
					LED_3_ON();
				}
				else
				{
					LED_3_OFF();
				}
				if (UartBuf[8]&0x08)
				{
					LED_4_ON();
				}
				else
				{
					LED_4_OFF();
				}
				// if (UartBuf[8]&0x10)
				// {
				// 	LED_5_ON();
				// }
				// else
				// {
				// 	LED_5_OFF();
				// }
				// if (UartBuf[8]&0x20)
				// {
				// 	LED_T_ON();
				// }
				// else
				// {
				// 	LED_T_OFF();
				// }
				BatStatus.Bits.DisOC = 0;
				BatStatus.Bits.DisOCL = 0;
				BatStatus.Bits.DisSC = 0;
				BatStatus.Bytes.HardFault = 0;

				if ((UartBuf[9]>0) && (UartBuf[9]<60))
				{
					AteForceCtrlTimer = (u16)UartBuf[9]*100;	// 60S以内
				}
				else
				{
					AteForceCtrlTimer = ATE_FORCE_CTRL_DELAY;
				}
				UartBuf[6] = 1;										// 数据1# =1表示请求确认
			}
			else
			{
				UartBuf[6] = 0;										// 非测试模式下,不进行控制
			}
			
			UartBuf[5] = 1;											// 数据长度 1个
		}
		break;
		
		// 读取综合信息查询
		case COMM_ATE_GENERAL_INFO:
		{					
			j = 6;
			UartBuf[j++] = AteGeneralInfoLenth.Qr;
			UartBuf[j++] = AteGeneralInfoLenth.SoftVer;
			UartBuf[j++] = AteGeneralInfoLenth.HardVer;
			UartBuf[j++] = AteGeneralInfoLenth.Vol;
			UartBuf[j++] = AteGeneralInfoLenth.DisCur;
			UartBuf[j++] = AteGeneralInfoLenth.ChgCur;
			UartBuf[j++] = AteGeneralInfoLenth.Ntc;
			UartBuf[j++] = AteGeneralInfoLenth.Status;
			
			memcpy((uint8_t *)&UartBuf[6+GNR_INF_NUM], DataFlashAtOnceSave.QRCode, AteGeneralInfoLenth.Qr);
			memcpy((uint8_t *)&UartBuf[6+GNR_INF_NUM+GNR_INF_QR_LEN], SW_VERSION, AteGeneralInfoLenth.SoftVer);
			memcpy((uint8_t *)&UartBuf[6+GNR_INF_NUM+GNR_INF_QR_LEN+GNR_INF_SW_LEN], HW_VERSION, AteGeneralInfoLenth.HardVer);
			j = 6+GNR_INF_NUM+GNR_INF_QR_LEN+GNR_INF_SW_LEN+GNR_INF_HW_LEN;

			UartBuf[j++] = (u8)(BatData.VolTotal/10);
			UartBuf[j++] = (u8)((BatData.VolTotal/10)>>8);
			UartBuf[j++] = (u8)(ChgAvgVol/10);
			UartBuf[j++] = (u8)((ChgAvgVol/10)>>8);			
			UartBuf[j++] = (u8)(ChgCurZero>>0);
			UartBuf[j++] = (u8)(ChgCurZero>>8);
			UartBuf[j++] = (u8)(ChgCurAvgAD>>0);
			UartBuf[j++] = (u8)(ChgCurAvgAD>>8);
			
			for (i=0;i<CELL_NUM;i++)
			{
				UartBuf[j+i*2] = (u8)(BatData.Bat[i].Vol);
				UartBuf[j+i*2+1] = (u8)(BatData.Bat[i].Vol>>8);
				
			}
			j += 2*CELL_NUM;
			
			UartBuf[j++] = (u8)(DisCurAvg/10);
			UartBuf[j++] = (u8)((DisCurAvg/10)>>8);	
			
			UartBuf[j++] = (u8)(ChgCurAvg/10);
			UartBuf[j++] = (u8)((ChgCurAvg/10)>>8);
			Data16 = ChgCurAvgAD*OPA_CUR_GAIN;
			UartBuf[j++] = (u8)(Data16/10);
			UartBuf[j++] = (u8)((Data16/10)>>8);
			
			UartBuf[j++] = (u8)(BatNtcTemp[0]);
			UartBuf[j++] = (u8)(BatNtcTemp[0]>>8);
			UartBuf[j++] = (u8)(BatNtcTemp[1]);
			UartBuf[j++] = (u8)(BatNtcTemp[1]>>8);
//			UartBuf[j++] = (u8)(BatNtcTemp[2]);
//			UartBuf[j++] = (u8)(BatNtcTemp[2]>>8);
			UartBuf[j++] = (u8)(DmosNtcTemp);
			UartBuf[j++] = (u8)(DmosNtcTemp>>8);
			UartBuf[j++] = (u8)(CmosNtcTemp);
			UartBuf[j++] = (u8)(CmosNtcTemp>>8);

			UartBuf[j] = AFE_ALTN_VALID();
			UartBuf[j] <<= 1;
		#ifdef KEY_TO_LED
			UartBuf[j] |= KEY_LED_PRESS();
		#endif
			UartBuf[j] <<= 1;
			UartBuf[j] |= VLOCK_IN();
			j++;
			
			UartBuf[j++] = BatStatus.Bytes.DisProt;
			UartBuf[j++] = BatStatus.Bytes.ChgProt;
			UartBuf[j++] = BatStatus.Bytes.HardFault;
			UartBuf[j++] = BatStatus.Bytes.BmsState;	
			UartBuf[j++] = BatStatus.Bytes.InputState;
			
			// UartBuf[j++] = AfeState.Bytes.Status1L;
			// UartBuf[j++] = AfeState.Bytes.Status1H;
			// UartBuf[j++] = AfeState.Bytes.Status2L;
			// UartBuf[j++] = AfeState.Bytes.Status2H;
			// UartBuf[j++] = AfeState.Bytes.State1L;
			// UartBuf[j++] = AfeState.Bytes.State1H;
			
			UartBuf[5] = AteGeneralInfoLenth.SumLen;					// 数据长度 2个
		}
		break;
		default:
			break;
	}
}
#endif

/*****************************************************************************
 函 数 名  : UartIsTimeOut
 功能描述  : UART超时检测
 输入参数  : void
 返 回 值  :
*****************************************************************************/
#if 0
u8 UartIsTimeOut(void)
{
    /* Uart Ticks: 1 ms */
    if(Rx_Flag || Tx_Flag)
    {
        Uart_TimeoutCount++;
    }
    else
    {
        Uart_TimeoutCount = 0;
    }

    if(Uart_TimeoutCount < UART_TIMEOUT_MAX)
        return 0;

    Rx_Flag = 0;

    Tx_Flag = 0;
    UartBmsIntTxBufLen = 0;

    return 1;
}
#endif
/*****************************************************************************
 函 数 名  : UartTxExit
 功能描述  : UART采用中断发送模式时，发送停止后执行发送模块禁止
 输入参数  : void
 返 回 值  :
*****************************************************************************/
#if 0
static  void UartTxExit(void)
{
    static XRAM u8 uart_tx_enable = 0;

    if(Tx_Flag)
    {
        uart_tx_enable = 1;
        return;
    }

    if(uart_tx_enable)
    {
//        UartRxEnable();
        uart_tx_enable = 0;
    }
}
#endif
/*****************************************************************************
 函 数 名  : UartCtrl
 功能描述  : 通讯模块总接口
 输入参数  :
 返 回 值  :
*****************************************************************************/
void UartBwCtrl(void)
{
    /* Uart Ticks: 10 ms */
    if(!TimerUartBWFlag)
        return;
    TimerUartBWFlag = 0;
    
//  UartTxExit();
//  UartIsTimeOut();        // message time out check

    UartCmdAct();           // BW protocol reply
}

/*****************************************************************************
 函 数 名  : UartCmdAct
 功能描述  : Uart通讯命令处理接口
 输入参数  : void
 返 回 值  :
*****************************************************************************/
void UartCmdAct(void)
{
    u8 i;
    XRAM u8 checksum;
    XRAM u8 temp;
    XRAM u8 flag;
    static XRAM u8 cmd_delay_timer = 0;     /* 命令回复延时计时器 */

    if((UartCmdActFlag == 0) || Tx_Flag)
    {
        return;
    }

    /* 命令回复适当做点延迟处理，确保单线模式下通讯正常 */
    cmd_delay_timer++;
    if(cmd_delay_timer < UART_CMD_DELAY)
        return;
    cmd_delay_timer = 0;

    Rx_Flag = 0;

    if(UartDataParse() == 1)                                        // 无效数据: 接收数据CheckSum计算&校验
    {
        UartLen = 0;
        Rx_UartLen = 0;

        for(i = 0; i < sizeof(UartBuf); i++)
        {
            UartBuf[i] = 0;
        }

        UartCmdActFlag = 0;                                         // 防止正常接收数据完成后,未处理就被清除 Neiyang->2018.05.09
        return;
    }

    checksum = 0;
    UartLen = 0;
    UartBuf[0] = DATA_END;                                          // 0xF2
    temp = UartBuf[1];                                              // 记录之前查询的发送地址
    UartBuf[1] = ID_BMS;                                            // 应答的发送地址
    UartBuf[2] = temp;                                              // 应答的接收地址
    temp = UartBuf[3];                                              // 功能码
    flag = temp & 0x01;                                             // 读/写操作:  '0'-写 '1'-读
    temp = temp & 0xFE;

    switch(temp)                                                    // 功能码判断
    {
        case FUNC_PRODUCE:                                          // 0x10 生产信息类
            FuncProduce(flag);
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
            FuncCalib(flag);
            break;

        case FUNC_SETTING:                                          // 0x80 配置管理类
            FuncSetting();
            break;
	#if 1
        case FUNC_ATE_INFO:
			FuncATE(flag);
			break;
	#endif
        default:
            break;
    }

    UartLen = 5 + UartBuf[5];

    for(i = 1; i <= UartLen; i++)
    {
        checksum += UartBuf[i];
    }

    UartBuf[UartLen + 1] = checksum;
    UartBuf[UartLen + 2] = DATA_HEAD;
    UartLen += 3;

#if 0
	for(i=0; i<UartLen; i++)
	{		
		UartBmsSendByte(*(UartBuf+i));		// polling send bytes		
	}
	UartLen = 0;
    Rx_UartLen = 0;
	UartCmdActFlag = 0;
#else
	UartBmsStartSend((u8 *)UartBuf, UartLen);
	UartLen = 0;
	Rx_UartLen = 0;
#endif
}
