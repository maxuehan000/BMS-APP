/******************************************************************************

                  版权所有 (C), 2001-2016, 惠州市蓝微电子有限公司

 ******************************************************************************
  文 件 名   : McuHal_N32G030.h
  版 本 号   : 初稿
  作    者   : 1
  生成日期   : 2020年8月14日 星期五
  最近修改   :
  功能描述   : MCUHAL_N32G45X.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年8月14日 星期五
    作    者   : 1
    修改内容   : 创建文件

******************************************************************************/
#ifndef __MCUHAL_N32G45X_H__
#define __MCUHAL_N32G45X_H__

#include "Board.h"
#include "SysDefs.h"


#define FLASH_BASE_ADDR					0x08000000
#define FLASH_PAGE_SIZE					2048
#define FLASH_DATA_BASE_ADDR			(1024UL*(512-8)+FLASH_BASE_ADDR)
#define FLASH_DATA_SIZE					(FLASH_PAGE_SIZE*4)

#define BOOT_LOADER_SIZE				0x2000							// 8K
#define UPDATE_FILE_MAX_LEN 			0x3E000    						// 240K, 长度不超过 =(总长512K-Boot8k-DataFlash4K)/2=58K
#define UPDATE_OFFEST_ADDR  			UPDATE_FILE_MAX_LEN   			// 备份区和app区的相对偏移地址
#define UPDATE_BASE_ADDR				(FLASH_BASE_ADDR+BOOT_LOADER_SIZE+UPDATE_FILE_MAX_LEN)


typedef enum
{
	SW_I2C0 = 0,
	SW_I2C1 = 1,
}SW_I2C_DEV;

typedef enum
{
	BW_DISABLE = 0,
	BW_ENABLE = !BW_DISABLE
}BW_ABLE_CMD;

extern void ADCInit(void);
extern void ADCDeInit(void);
//extern u16 ADRead(u8 channel);
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
extern void HwFailSafeInit(void);
extern void HwMcuReset(void);
extern void PortSWDInit(void);
extern void UartRxDisable(void);
extern void UartRxEnable(void);
extern void UartSendByte(USART_Module* uartx, u8 tx_data);
extern void UartSendByteIT(USART_Module* uartx, u8 tx_data);
extern u8 UartReceiveByte(void);

extern void I2CInit(void);
extern u8 I2C_ReadBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length);
extern u8 I2C_WriteBuffer(u8 DevI2C, u8 addr, u8 cmdCode, u8 *pBuf, u8 length);
extern u8 I2C_ReadBuffer_led(u8 DevI2C, u8 cmdCode, u8 *pBuf, u8 length);
extern u8 I2C_WriteBuffer_led(u8 DevI2C, u8 cmdCode, u8 *pBuf, u8 length);



extern u8 SpiTransmitBuffer(SPI_Module* SpiDev, u8 NumSend, u8 *Sbuf,u8 NumRec, u8 *Rbuf);


extern void HwSleepEnter(void);
extern void HwSleepExit(void);
extern void HwSleepWkupToggle(BW_ABLE_CMD cmd);

#define SLEEP_NOW      0
#define SLEEP_ON_EXIT  1

inline void HwStopMode(void)
{
	PWR_EnterSLEEPMode(SLEEP_NOW, PWR_STOPENTRY_WFI);
	//PWR_EnterStopState(PWR_REGULATOR_LOWPOWER, PWR_STOPENTRY_WFI);
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP();    			// 退出休眠时增加nop延时后再进行喂狗
}


#endif /* __MCUHAL_N32G45X_H__ */

