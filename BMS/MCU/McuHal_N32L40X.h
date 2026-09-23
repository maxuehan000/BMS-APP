/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : McuHal_N32G030.h
  版 本 号   : 初稿
  作    者   : 1
  生成日期   : 2020年8月14日 星期五
  最近修改   :
  功能描述   : MCUHAL_N32L40X.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年8月14日 星期五
    作    者   : 1
    修改内容   : 创建文件

******************************************************************************/
#ifndef __MCUHAL_N32L40X_H__
#define __MCUHAL_N32L40X_H__

#include "Board.h"

#define FLASH_BASE_ADDR					0x08000000
#define FLASH_PAGE_SIZE					2048
#define FLASH_DATA_BASE_ADDR			(1024UL*(128-4)+FLASH_BASE_ADDR)
#define FLASH_DATA_SIZE					(FLASH_PAGE_SIZE*2)


#define BOOT_LOADER_SIZE		0x2000					// 8K
#define UPDATE_FILE_MAX_LEN 	0xE000    				// 56K, 长度不超过 =(总长128K-Boot8k-DataFlash4K)/2=58K
#define UPDATE_OFFEST_ADDR  	UPDATE_FILE_MAX_LEN   	// 备份区和app区的相对偏移地址
#define UPDATE_BASE_ADDR		(FLASH_BASE_ADDR+BOOT_LOADER_SIZE+UPDATE_FILE_MAX_LEN)


typedef enum
{
	SW_I2C0 = 0,
	SW_I2C1 = 1,
}SW_I2C_DEV;

extern void ADCDeInit(void);
extern u16 ADRead(u8 channel);
extern void Delay10us(u32 uldelay);
extern void Delay10us_INT(u32 uldelay);
extern void Delay1ms(u16 TimeTick);
extern void HwClrWdt(void);
extern void HwDataInit(void);
extern void HwDataLock(void);
extern void HwDataUnlock(void);
extern u8 HwDataRead(u32 Addr, u32 *pData, u16 Len);
extern u8 HwDataReadRandom(u32 Addr, u32 *pData, u16 Len);
extern u8 HwDataPageWrite(u32 Addr, u32 *pData, u16 Len);
extern u8 HwDataPageWriteRandom(u32 Addr, u32 *pData, u16 Len);
extern void HwIntDisable(void);
extern void HwIntEnable(void);
extern void HwMcuInit(void);
extern void HwMcuReset(void);
extern void UartRxDisable(void);
extern void UartRxEnable(void);
extern void UartSendByte(u8 tx_data);
extern void UartSendByteIT(u8 tx_data);
extern u8 UartReceiveByte(void);

extern void I2CInit(void);
extern u8 I2C_ReadBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length);
extern u8 I2C_WriteBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length);

extern u8 SpiTransmitBuffer(SPI_Module* SpiDev, u8 NumSend, u8 *Sbuf,u8 NumRec, u8 *Rbuf);

#ifdef USE_MCU_RTC
#pragma pack(1)
typedef struct
{
    uint16_t year	: 8;    				// 2000年基数，eg.2024年=24
    uint8_t month	: 4;
    uint8_t week 	: 4;
    uint8_t day		: 8;   
    uint8_t hour		: 6;				
    uint8_t ampm		: 1;				// 12h制:am=0,pm=1, 24h制:=0		
    uint8_t hour_12_24	: 1;				// 仅支持默认24h制=0 !!!
    uint8_t minute	: 8;
    uint8_t second	: 8;    
}RTC_TIME_S;
#pragma pack()

extern void RtcTimeSet(RTC_TIME_S *RTC_Time);
extern void RtcTimeRead(RTC_TIME_S *RTC_Time);
#endif

extern void HwSleepEnter(void);
extern void HwSleepExit(void);

void HwStopMode(void);

#endif /* __MCUHAL_N32L40X_H__ */

